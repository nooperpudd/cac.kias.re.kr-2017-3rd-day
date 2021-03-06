#include <math.h>
#include <string.h>

#define n 4096
#define m 4096

double A[n][m];
double Anew[n][m];

int main(int argc, char** argv)
{
    const int iter_max = 1000;
    
    const double tol = 1.0e-6;
    double error     = 1.0;
    
    memset(A, 0, n * m * sizeof(double));
    memset(Anew, 0, n * m * sizeof(double));
        
    for (int j = 0; j < n; j++)
    {
        A[j][0]    = 1.0;
        Anew[j][0] = 1.0;
    }
    
    printf("Jacobi relaxation Calculation: %d x %d mesh\n", n, m);
    
    int iter = 0;

#pragma acc data copy(A), create(Anew)
    while ( error > tol && iter < iter_max )
    {
        error = 0.0;

#pragma omp parallel for shared(Anew, A)
#pragma acc kernels loop
        for( int j = 1; j < n-1; j++)
        {
#pragma acc loop gang(16) vector(32)
            for( int i = 1; i < m-1; i++ )
            {
                Anew[j][i] = 0.25 * ( A[j][i+1] + A[j][i-1]
                                    + A[j-1][i] + A[j+1][i]);
                error = fmax( error, fabs(Anew[j][i] - A[j][i]));
            }
        }
        
#pragma omp parallel for shared(Anew, A)
#pragma acc kernels loop
        for( int j = 1; j < n-1; j++)
        {
#pragma acc loop gang(16) vector(32)
            for( int i = 1; i < m-1; i++ )
            {
                A[j][i] = Anew[j][i];    
            }
        }
        
        if(iter % 100 == 0) printf("%5d, %0.6f\n", iter, error);
        
        iter++;
    }
return 0;
} 

