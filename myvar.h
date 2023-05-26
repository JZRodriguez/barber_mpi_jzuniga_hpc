#ifndef MYVAR
#define MYVAR

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


typedef struct{
	double a;
	double b;
	double dx;
	double F; //result of the integral
}MPI_myvar;


#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* myvar */
