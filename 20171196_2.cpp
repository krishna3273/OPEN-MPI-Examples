#include<mpi.h>
#include<time.h>
#include <fstream>
#include <bits/stdc++.h> 
#include <cmath> 
using namespace std;
void inline swap(int&a,int&b){
    int temp=a;
    a=b;
    b=temp;
    return;
}

int  partition(int arr[],int low,int high,int pivot){
    int curr=low;
    for(int i=low;i<high;i++){
        if(arr[i]<=pivot){
            swap(arr[i],arr[curr]);
            curr++;
        }
    }
    swap(arr[curr],arr[high]);
    return curr;
}
void qsort(int arr[],int low,int high){
    // int n=high-low+1;
    if(low>=high) return;
    srand ( time(NULL) );
    int rand_ind=low+rand()%(high-low+1);
    int pivot=arr[rand_ind];
    swap(arr[high],arr[rand_ind]);
    int ind=partition(arr,low,high,pivot);
    qsort(arr,low,ind-1);
    qsort(arr,ind+1,high);
    return;
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
        out<<n<<"\n";
        int input[n];
        for(int i=0;i<n;i++){
            is >> input[i];
        }
        // for(int i=0;i<n;i++) cout<<input[i]<<" ";
        // cout<<endl;
        int start=0;
        int step=ceil((float)n/(float)size);
        // if(size==1){
            
        //     MPI_Finalize(); 
        //     return EXIT_SUCCESS;
        // }
        
        start+=step;
        for(int i=1;i<size;i++){
            if(start<=n-1){
                int s=min(step,n-start);
                MPI_Send(&s,1,MPI_INT,i,0,MPI_COMM_WORLD);
                MPI_Send(input+start,s,MPI_INT,i,0,MPI_COMM_WORLD);
                start+=s;
            }
            else{
                int a=-1;
                MPI_Send(&a,1,MPI_INT,i,0,MPI_COMM_WORLD);
            }
        }
        qsort(input,0,step-1);
        start=step;
        int starts[size];
        int sizes[size];
        starts[0]=0;
        sizes[0]=step;
        priority_queue<pair<int, pair<int, int> > , vector<pair<int, pair<int, int> > >, greater<pair<int, pair<int, int> > > > pq;
        for(int i=1;i<size;i++){
            int s;
            MPI_Recv(&s,1,MPI_INT,i,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            starts[i]=starts[i-1]+sizes[i-1];
            sizes[i]=s;
            if(s<=0) continue;
            MPI_Recv(input+starts[i], s, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        for (int i = 0; i <size; i++){
            if(sizes[i]>0)
                pq.push({ input[starts[i]], { i, starts[i] } });
        } 
  
        while (pq.empty() == false) { 
            pair<int, pair<int, int> >  curr = pq.top(); 
            pq.pop(); 
    
        
            int i = curr.second.first; 
            int j = curr.second.second; 
    
            out<<curr.first<<" "; 
    
        
            if (j + 1-starts[i] < sizes[i]) 
                pq.push({ input[j + 1], { i, j + 1 } }); 
        } 
    }
    else{
        int s;
        MPI_Recv(&s,1,MPI_INT,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        // cout<<s<<endl;
        if(s<=0){
            s=-1;
            MPI_Send(&s,1,MPI_INT,0,0,MPI_COMM_WORLD);
        }
        else{
            int temp[s];
            MPI_Recv(temp, s, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            // for(int i=0;i<s;i++) cout<<temp[i]<<" ";
            // cout<<endl;
            qsort(temp,0,s-1);
            // for(int i=0;i<s;i++) cout<<temp[i]<<" ";
            // cout<<endl;
            MPI_Send(&s,1,MPI_INT,0,0,MPI_COMM_WORLD);
            MPI_Send(temp, s, MPI_INT, 0, 0, MPI_COMM_WORLD);
        }
    }

    MPI_Finalize(); 
    return EXIT_SUCCESS;
}
