#include <string.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <check.h>
#include <sasalib.h>
#if HAVE_CONFIG_H
#  include <config.h>
#endif

#ifndef PI
#define PI 3.14159265358979323846
#endif

double rel_err(double v1, double v2) {
    return fabs(v1-v2)/(fabs(v1)+fabs(v2));
}

void alg_failed(const char *alg,const char *test)
{
    printf("%s failed test: %s\n",alg,test);
}

double surface_hidden_sphere_intersection(double r1, double r2, double d) 
{
    if (d > r1 + r2) return 0;
    if (r1+d < r2) return 4*PI*r1*r1;
    if (r2+d < r1) return 4*PI*r2*r2;
    return PI/d * ( r1*(r2*r2 - (d-r1)*(d-r1))
		   +r2*(r1*r1 - (d-r2)*(d-r2)) ) ;
}
double surface_spheres_intersecting(double r1, double r2, double d)
{
    return 4*PI*(r1*r1+r2*r2) - surface_hidden_sphere_intersection(r1,r2,d);
}

double surface_two_spheres(const double *x, const double *r, double probe)
{
    double d2 = (x[0]-x[3])*(x[0]-x[3]) + (x[1]-x[4])*(x[1]-x[4]) 
	+ (x[2]-x[5])*(x[2]-x[5]);
    return surface_spheres_intersecting(r[0]+probe,r[1]+probe,sqrt(d2));
}

int test_sasa(sasalib_t *s, double ref, double tolerance,const char *test)
{
    sasalib_refresh(s);
    double err;
    if ((err = rel_err(ref,sasalib_area_total(s)))
	> tolerance) {
	alg_failed(sasalib_algorithm_name(s),test);
	printf("%f %f\n",ref,sasalib_area_total(s));
	return 1;
    }
    return 0;
}

int test_sasa_alg_basic(sasalib_t *s, double tolerance) 
{
    int err = 0;
    int nt = 0;
    
    // Two spheres, compare with analytic results
    double coord[6] = {0,0,0,2,0,0};
    double r[2] = {1,2};
    double probe = sasalib_get_probe_radius(s);
    sasalib_link_coord(s,coord,r,2);
    
    ++nt;
    err += test_sasa(s,surface_two_spheres(coord,r,probe),tolerance,
		     "Two intersecting spheres along x-axis.");
    coord[3] = 0; coord[4] = 2;
    ++nt;
    err += test_sasa(s,surface_two_spheres(coord,r,probe),tolerance,
		     "Two intersecting spheres along y-axis."); 
    coord[4] = 0; coord[5] = 2;
    ++nt;
    err += test_sasa(s,surface_two_spheres(coord,r,probe),tolerance,
		     "Two intersecting spheres along z-axis.");

    // Four spheres in a plane, all calculations should give similar results
    double coord2[12] = {0,0,0, 1,0,0, 0,1,0, 1,1,0};
    double r2[4] = {1,1,2,1};
    sasalib_link_coord(s,coord2,r2,4);
    sasalib_refresh(s);
    double ref = sasalib_area_total(s);

    //translate
    for (int i = 0; i < 12; ++i) coord2[i] += 1.;
    ++nt;
    err += test_sasa(s,ref,tolerance,"Four spheres in plane, translated");

    //rotate 90 degrees round z-axis
    double coord3[12] = {0,1,0, 0,0,0, 1,1,0, 1,0,0};
    memcpy(coord2,coord3,12*sizeof(double));
    ++nt;
    err += test_sasa(s,ref,tolerance,"Four spheres in plane, rotated 90 deg round z-axis.");

    //rotate -45 degrees round z-axis
    double sqr2 = sqrt(2);
    double coord4[12] = {-1./sqr2,1./sqr2,0, 0,0,0, 0,sqr2,0, 1/sqr2,1/sqr2,0};
    memcpy(coord2,coord4,12*sizeof(double));
    ++nt;
    err += test_sasa(s,ref,tolerance,"Four spheres in plane, rotated 45 deg round z-axis.");

    //rotate 90 degrees round x-axis
    double coord5[12] = {-1./sqr2,0,1/sqr2, 0,0,0, 0,0,sqr2, 1/sqr2,0,1/sqr2};
    memcpy(coord2,coord5,12*sizeof(double));
    ++nt;
    err += test_sasa(s,ref,tolerance,"Four spheres in plane, rotated 90 deg round x-axis.");

    return err;
}

START_TEST (test_sasa_basic)
{
    sasalib_t *sr = sasalib_init();
    sasalib_t *lr = sasalib_init();

    sasalib_set_algorithm(sr,SASALIB_SHRAKE_RUPLEY);
    sasalib_set_algorithm(lr,SASALIB_LEE_RICHARDS);

    sasalib_set_sr_points(sr,5000);
    sasalib_set_lr_delta(lr,1e-4);

    ck_assert(test_sasa_alg_basic(sr,1e-3) == 0);
    ck_assert(test_sasa_alg_basic(lr,1e-5) == 0);

    sasalib_free(sr);
    sasalib_free(lr);
}
END_TEST

START_TEST (test_sasa_1ubq_sr)
{
    errno = 0;
    sasalib_t *sr = sasalib_init();
    sasalib_set_algorithm(sr,SASALIB_SHRAKE_RUPLEY);
    sasalib_set_sr_points(sr,100);
    FILE *pdb = fopen("data/1ubq.pdb","r");
    if (pdb == NULL) {
	fprintf(stderr,"error reading PDB-file for test. "
		"(Tests must be run from directory test/): %s\n",
		strerror(errno));
    }
    ck_assert(pdb != NULL);
    ck_assert(sasalib_calc_pdb(sr,pdb) == SASALIB_SUCCESS); 
    fclose(pdb);
    // The reference values were the output of Sasalib on 2014-02-10
    ck_assert(fabs(sasalib_area_total(sr) - 4756.124034) < 1e-5);
    ck_assert(fabs(sasalib_area_class(sr,SASALIB_POLAR) - 1968.057001) < 1e-5);
    ck_assert(fabs(sasalib_area_class(sr,SASALIB_APOLAR) - 2788.067033) < 1e-5);
    sasalib_free(sr);
}
END_TEST

START_TEST (test_sasa_1ubq_lr)
{
    sasalib_t *lr = sasalib_init();
    sasalib_set_algorithm(lr,SASALIB_LEE_RICHARDS);
    sasalib_set_lr_delta(lr,0.25);
    FILE *pdb = fopen("data/1ubq.pdb","r");
    if (pdb == NULL) {
	fprintf(stderr,"error reading PDB-file for test. "
		"(Tests must be run from directory test/): %s\n",
		strerror(errno));
    }
    ck_assert(pdb != NULL);
    ck_assert(sasalib_calc_pdb(lr,pdb) == SASALIB_SUCCESS); 
    fclose(pdb);
    // The reference values were the output of Sasalib on 2014-02-10
    ck_assert(fabs(sasalib_area_total(lr) - 4725.173153) < 1e-5);
    ck_assert(fabs(sasalib_area_class(lr,SASALIB_POLAR) - 1957.575594) < 1e-5);
    ck_assert(fabs(sasalib_area_class(lr,SASALIB_APOLAR) - 2767.597560) < 1e-5);
    sasalib_free(lr);
}
END_TEST

START_TEST (test_sasalib_api_basic)
{
    extern int sasalib_set_verbosity();
    sasalib_set_verbosity(1);
    sasalib_t *s = sasalib_init();
    ck_assert(s != NULL);
    ck_assert(sasalib_n_atoms(s) == 0);

    // algorithm
    ck_assert(sasalib_set_algorithm(s,-1) == SASALIB_WARN);
    ck_assert(sasalib_set_algorithm(s,1000) == SASALIB_WARN);
    ck_assert(sasalib_set_algorithm(s,SASALIB_LEE_RICHARDS) == SASALIB_SUCCESS);
    ck_assert(sasalib_get_algorithm(s) == SASALIB_LEE_RICHARDS);
    
    // probe_radius
    ck_assert(sasalib_set_probe_radius(s,-1.) == SASALIB_WARN);
    ck_assert(sasalib_set_probe_radius(s,1.2) == SASALIB_SUCCESS);
    ck_assert(fabs(sasalib_get_probe_radius(s)-1.2) < 1e-10);
    
    // L&R delta
    double lrd_def = sasalib_get_lr_delta(s);
    ck_assert(sasalib_set_lr_delta(s,0.5) == SASALIB_SUCCESS);
    ck_assert(fabs(sasalib_get_lr_delta(s)-0.5) < 1e-10);
    ck_assert(sasalib_set_lr_delta(s,-1.0) == SASALIB_WARN);
    ck_assert(fabs(sasalib_get_lr_delta(s)-lrd_def) < 1e-10);
    ck_assert(sasalib_get_sr_points(s) == SASALIB_WARN);
    
    // S&R test-points
    ck_assert(sasalib_set_algorithm(s,SASALIB_SHRAKE_RUPLEY) == SASALIB_SUCCESS);
    int srp_def = sasalib_get_sr_points(s);
    ck_assert(sasalib_set_sr_points(s,100) == SASALIB_SUCCESS);
    ck_assert(sasalib_get_sr_points(s) == 100);
    ck_assert(sasalib_set_sr_points(s,1123) == SASALIB_WARN);
    ck_assert(sasalib_set_sr_points(s,-1123) == SASALIB_WARN);
    ck_assert(sasalib_get_sr_points(s) == srp_def);
    ck_assert(sasalib_get_lr_delta(s) < 0);

    // names
    sasalib_set_proteinname(s,"bla");
    ck_assert_str_eq(sasalib_get_proteinname(s),"bla");
	     
#if HAVE_LIBPTHREAD
    // Threads
    int nt_def = sasalib_get_nthreads(s);
    ck_assert(sasalib_set_nthreads(s,2) == SASALIB_SUCCESS);
    ck_assert(sasalib_get_nthreads(s) == 2);
    ck_assert(sasalib_set_nthreads(s,-1) == SASALIB_WARN);
    ck_assert(sasalib_get_nthreads(s) == nt_def);
#endif    
    
    // Check that results cannot be accessed before calculations are
    // performed
    ck_assert(sasalib_area_total(s) < 0);
    ck_assert(sasalib_area_class(s, SASALIB_POLAR) < 0);
    ck_assert(sasalib_area_class(s, SASALIB_APOLAR) < 0);
    ck_assert(sasalib_per_residue(stdout,s) == SASALIB_FAIL);
    ck_assert(sasalib_per_residue(NULL,s) == SASALIB_FAIL);
    ck_assert(sasalib_area_residue(s,"ALA") < 0);
    ck_assert(sasalib_write_pdb(stdout,s) == SASALIB_FAIL);
    ck_assert(sasalib_area_atom(s,0) < 0);
    ck_assert(sasalib_area_atom_array(s) == NULL);

    ck_assert(sasalib_log(stdout,s) == SASALIB_WARN);
    sasalib_set_verbosity(0);
}
END_TEST

START_TEST (test_multi_calc) 
{
#if HAVE_LIBPTHREAD
    errno = 0;
    sasalib_t *s = sasalib_init();
    //S&R
    sasalib_set_algorithm(s,SASALIB_SHRAKE_RUPLEY);
    sasalib_set_sr_points(s,100);
    sasalib_set_nthreads(s,2);
    FILE *pdb = fopen("data/1ubq.pdb","r");
    if (pdb == NULL) {
	fprintf(stderr,"error reading PDB-file for test. "
		"(Tests must be run from directory test/): %s\n",
		strerror(errno));
    }
    ck_assert(pdb != NULL);
    ck_assert(sasalib_calc_pdb(s,pdb) == SASALIB_SUCCESS); 
    // The reference values were the output of Sasalib on 2014-02-10
    ck_assert(fabs(sasalib_area_total(s) - 4756.124034) < 1e-5);
    // L&R
    sasalib_set_algorithm(s,SASALIB_LEE_RICHARDS);
    sasalib_set_lr_delta(s,0.25);
    rewind(pdb);
    ck_assert(sasalib_calc_pdb(s,pdb) == SASALIB_SUCCESS); 
    ck_assert(fabs(sasalib_area_total(s) - 4725.173153) < 1e-5);
    fclose(pdb);
#endif
}
END_TEST


Suite *sasa_suite() 
{
    Suite *s = suite_create("SASA-calculation");
    TCase *tc_basic = tcase_create("Basic");
    TCase *tc_1ubq = tcase_create("1UBQ");
    tcase_add_test(tc_basic, test_sasalib_api_basic);
    tcase_add_test(tc_basic, test_sasa_basic);
    tcase_add_test(tc_1ubq, test_sasa_1ubq_sr);
    tcase_add_test(tc_1ubq, test_sasa_1ubq_lr);
    suite_add_tcase(s, tc_basic);
    suite_add_tcase(s, tc_1ubq);
#if HAVE_LIBPTHREAD
    printf("Using pthread\n");
    TCase *tc_pthr = tcase_create("Pthread");
    tcase_add_test(tc_pthr,test_multi_calc);
    suite_add_tcase(s, tc_pthr);
#endif
    return s;
}
