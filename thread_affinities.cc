#include <stdio.h>
#include <mpi.h>
#include <numa.h>
#include <unistd.h>
#include <omp.h>

int main ( int argc, char ** argv )
{
    int err, nprocs, proc_id;
    err = MPI_Init( &argc, &argv );
    err = MPI_Comm_size( MPI_COMM_WORLD, &nprocs );
    err = MPI_Comm_rank( MPI_COMM_WORLD, &proc_id );

    char hostname[1024];
    err = gethostname( &hostname[0], 1024 * sizeof(char) );

#pragma omp parallel shared(hostname, nprocs, proc_id)
    {
        const int outer_id = omp_get_thread_num();
        //const int outer_nthreads = omp_get_max_threads();
        const int outer_nthreads = omp_get_num_threads();

#pragma omp parallel shared(outer_id, outer_nthreads)
        {
            const int inner_id = omp_get_thread_num();
            //const int inner_nthreads = omp_get_max_threads();
            const int inner_nthreads = omp_get_num_threads();

            struct bitmask * allowed_mems = numa_get_membind();
            const int num_nodes = numa_num_configured_nodes();
            int node_list[num_nodes];
            int cur_nodes = 0;

            for ( int i = 0; i < num_nodes; i++ )
            {
                if ( numa_bitmask_isbitset(allowed_mems, i) ) {
                    node_list[cur_nodes] = i;
                    cur_nodes++;
                }
            }

            const int perf_node = numa_preferred();
            const int num_cpus = numa_num_task_cpus();
            const int max_cpus = numa_num_possible_cpus();
            struct bitmask *mycpus = numa_allocate_cpumask();

            int set_ids = 0;
            int cpu_ids[max_cpus];

            // Ret should be # bytes copied
            int ret = numa_sched_getaffinity(0, mycpus);

            if ( ret <= 0 )
            {
                printf("Error getting affinity...");
                exit(1);
            }

            for ( int i = 0; i < max_cpus; i++ )
            {
                if ( numa_bitmask_isbitset(mycpus, i) )
                {
                    cpu_ids[set_ids] = i;
                    set_ids++;
                }
            }

            char cpu_list[4096];
            sprintf(cpu_list, " Local CPU List: ");
            for ( int i = 0; i < set_ids; i++ )
            {
                sprintf(cpu_list, "%s%03d ", cpu_list, cpu_ids[i]);
            }
            printf( "(R,T,T) = (%03d, %03d, %03d) of (%03d, %03d, %03d) is on hostname %s is using numa node %03d and has access to %03d of %03d cpus (%s)\n", proc_id, outer_id, inner_id, nprocs, outer_nthreads, inner_nthreads, hostname, perf_node, set_ids, num_cpus, cpu_list);
        }
    }

    err = MPI_Finalize();
}
