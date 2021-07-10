#include<mpi.h>
#include<time.h>
#include <fstream>
#include<iostream>
#include <cmath> 
using namespace std;

float sum(int start,int end){
    float ans=0;
    for(int i=start;i<=end;i++){
        float j=i;
        ans+=(1/(j*j));
    }
    return ans;
}
int main(int argc,char* argv[]){
    MPI_Init(&argc, &argv); 
    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    enum role_ranks { SENDER, RECEIVER }; 
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if(rank==0){
        string input_file=argv[1];
        string output_file=argv[2];
        // cout<<input_file<<endl;
        ifstream is(input_file);
        ofstream out;
        out.open(output_file);
        int n;
        is >> n;
        int start=1;
        int step=ceil((float)n/(float)size);
        // cout<<step<<endl;
        float res;
        float ans=0;
        for(int i=1;i<size;i++){
            int inputs[2]={start,min(start+step-1,n)};
            start+=step;
            MPI_Send(inputs,2,MPI_INT,i,0,MPI_COMM_WORLD);
        }
        for(int i=1;i<size;i++){
            MPI_Recv(&res, 1, MPI_FLOAT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            ans+=res;
        }
        ans+=sum(start,min(start+step-1,n));
        out<<ans;

    }
    else{
        int received[2];
        MPI_Recv(received, 2, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        int start=received[0];
        int end=received[1];
        float ans=sum(start,end);
        // cout<<start<<" "<<end<<" "<<ans<<endl;
        MPI_Send(&ans, 1, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize(); 
    return EXIT_SUCCESS;
}
