#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <iostream>
#include <mpi.h>

int main ( int argc, char *argv[] );
double boundary_condition ( double x, double time );
double initial_condition ( double x, double time );
double rhs ( double x, double time );
void timestamp ( );
void update ( int id, int p );

#define SIZE 100

int main(int argc, char *argv[]) {
	int world_size, world_rank;
	double itime, ftime, exec_time;

	MPI_Init(NULL, NULL);
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

	if(world_rank == 0) {
		itime = MPI_Wtime();
	}

	update(world_rank, world_size);

	if(world_rank == 0) {
		ftime = MPI_Wtime();
		std::cout << "Exec Time: " << ftime - itime << std::endl;
	}

	MPI_Finalize();
	return 0;	
}


void update ( int id, int p )
{
double cfl;

  double *h;
  FILE *h_file;
  double *h_new;
  int i;
  int j;
  int j_min = 0;
  int j_max = 400;
  double k = 0.002;
  int n = 11;
  int tag;
  double time;
  double time_delta;
  double time_max = 10.0;
  double time_min = 0.0;
  double time_new;
  double *x;
  double x_delta;
  FILE *x_file;
  double x_max = 1.0;
  double x_min = 0.0;

/*
  Have process 0 print out some information.
*/
if(id == 0) {
    printf ( "\n" );
    printf ( "  Compute an approximate solution to the time dependent\n" );
    printf ( "  one dimensional heat equation:\n" );
    printf ( "\n" );
    printf ( "    dH/dt - K * d2H/dx2 = f(x,t)\n" );
    printf ( "\n" );
    printf ( "  for %f = x_min < x < x_max = %f\n", x_min, x_max );
    printf ( "\n" );
    printf ( "  and %f = time_min < t <= t_max = %f\n", time_min, time_max );
    printf ( "\n" );
    printf ( "  Boundary conditions are specified at x_min and x_max.\n" );
    printf ( "  Initial conditions are specified at time_min.\n" );
    printf ( "\n" );
    printf ( "  The finite difference method is used to discretize the\n" );
    printf ( "  differential equation.\n" );
    printf ( "\n" );
    printf ( "  This uses %d equally spaced points in X\n", p * n );
    printf ( "  and %d equally spaced points in time.\n", j_max );
    printf ( "\n" );
    printf ( "  Parallel execution is done using %d processors.\n", p );
    printf ( "  Domain decomposition is used.\n" );
    printf ( "  Each processor works on %d nodes, \n", n );
    printf ( "  and shares some information with its immediate neighbors.\n" );
}
/*

 Set the X coordinates of the N nodes.
  We don't actually need ghost values of X but we'll throw them in
  as X[0] and X[N+1].
*/
  x = ( double * ) malloc ( ( n + 2 ) * sizeof ( double ) );

  for ( i = 0; i <= n + 1; i++ )
  {
    x[i] = ( ( double ) (         id * n + i - 1 ) * x_max
           + ( double ) ( p * n - id * n - i     ) * x_min )
           / ( double ) ( p * n              - 1 );
  }
/*
In single processor mode, write out the X coordinates for display.
*/
    x_file = fopen ( "x_data.txt", "w" );
    for ( i = 1; i <= n; i++ )
    {
      fprintf ( x_file, "  %f", x[i] );
    }
    fprintf ( x_file, "\n" );

    fclose ( x_file );
/*
  Set the values of H at the initial time.
*/
  time = time_min;
  h = ( double * ) malloc ( ( n + 2 ) * sizeof ( double ) );
  h_new = ( double * ) malloc ( ( n + 2 ) * sizeof ( double ) );
  h[0] = 0.0;
  for ( i = 1; i <= n; i++ )
  {
    h[i] = initial_condition ( x[i], time );
  }
  h[n+1] = 0.0;
  
  time_delta = ( time_max - time_min ) / ( double ) ( j_max - j_min );
  x_delta = ( x_max - x_min ) / ( double ) ( p * n - 1 );
/*
  Check the CFL condition, have processor 0 print out its value,
  and quit if it is too large.
*/
  cfl = k * time_delta / x_delta / x_delta;
    printf ( "\n" );
    printf ( "UPDATE\n" );
    printf ( "  CFL stability criterion value = %f\n", cfl );

if ( 0.5 <= cfl ) 
  {
      printf ( "\n" );
      printf ( "UPDATE - Warning!\n" );
      printf ( "  Computation cancelled!\n" );
      printf ( "  CFL condition failed.\n" );
      printf ( "  0.5 <= K * dT / dX / dX = %f\n", cfl );
    	return;
  }
/*

 In single processor mode, write out the values of H.
*/
    h_file = fopen ( "h_data.txt", "w" );

    for ( i = 1; i <= n; i++ )
    {
      fprintf ( h_file, "  %f", h[i] );
    }
    fprintf ( h_file, "\n" );

/*
  Compute the values of H at the next time, based on current data.
*/
  for ( j = 1; j <= j_max; j++ )
  {

    time_new = ( ( double ) (         j - j_min ) * time_max
               + ( double ) ( j_max - j         ) * time_min )
               / ( double ) ( j_max     - j_min );
/*
  Send H[1] to ID-1.
*/
 if(id > 0) {
  tag = 1;
  MPI_Send(&h[1], 1, MPI_DOUBLE, id-1, tag, MPI_COMM_WORLD);
 }

/*
  Receive H[N+1] from ID+1.
*/
  if(id < p-1) {
    tag = 1;
    MPI_Recv(&h[n+1], 1, MPI_DOUBLE, id+1, tag, MPI_COMM_WORLD, NULL);
  }

/*
  Send H[N] to ID+1.
*/
    if(id < p-1) {
      tag = 2;
      MPI_Send(&h[n], 1, MPI_DOUBLE, id+1, tag, MPI_COMM_WORLD);
    }

/*
  Receive H[0] from ID-1.
*/
  if(id > 0) {
    tag = 2;
    MPI_Recv(&h[0], 1, MPI_DOUBLE, id-1, tag, MPI_COMM_WORLD, NULL);
  }

/*
  Update the temperature based on the four point stencil.
*/
    for ( i = 1; i <= n; i++ )
    {
      h_new[i] = h[i] 
      + ( time_delta * k / x_delta / x_delta ) * ( h[i-1] - 2.0 * h[i] + h[i+1] ) 
      + time_delta * rhs ( x[i], time );
    }
/*
  H at the extreme left and right boundaries was incorrectly computed
  using the differential equation.  Replace that calculation by
  the boundary conditions.
*/
  if(id == 0) {
    h_new[1] = boundary_condition(x[1], time_new);
  }
  if(id == p-1) {
    h_new[n] = boundary_condition(x[n], time_new);
  }

/*
  Update time and temperature.
*/
    time = time_new;

    for ( i = 1; i <= n; i++ )
    {
      h[i] = h_new[i];
    }
/*
  In single processor mode, add current solution data to output file.
*/

  if(p == 1) {
    for(i = 0; i < n; i++) {
      fprintf(h_file, " %f", h[i]);
    }
    fprintf(h_file, "\n");
  }

  }

  if(p == 1) {
    fclose(h_file);
  }

  free ( h );
  free ( h_new );
  free ( x );

  return;
}

double boundary_condition ( double x, double time )
{
  double value;
/*
  Left condition:
*/
  if ( x < 0.5 )
  {
    value = 100.0 + 10.0 * sin ( time );
  }
  else
  {
    value = 75.0;
  }

  return value;
}

/* double initial_condition ( double x, double time )

  Parameters:

    Input, double X, TIME, the position and time.

    Output, double INITIAL_CONDITION, the value of the initial condition.
*/
double initial_condition ( double x, double time )
{
  double value;

  value = 95.0;

  return value;
}
/******************************************************************************/

double rhs ( double x, double time )

/******************************************************************************/
/*
  Parameters:

    Input, double X, TIME, the position and time.

    Output, double RHS, the value of the right hand side function.
*/
{
  double value;

  value = 0.0;

  return value;
}
/******************************************************************************/

void timestamp ( )
/*
 Parameters:

    None
*/
{
# define TIME_SIZE 40

  static char time_buffer[TIME_SIZE];
  const struct tm *tm;
  time_t now;

  now = time ( NULL );
  tm = localtime ( &now );

  strftime ( time_buffer, TIME_SIZE, "%d %B %Y %I:%M:%S %p", tm );

  printf ( "%s\n", time_buffer );

  return;
# undef TIME_SIZE
}