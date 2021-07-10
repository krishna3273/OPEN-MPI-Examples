#include<time.h>
#include<mpi.h>
#include <fstream>
#include<bits/stdc++.h>
#include <cmath> 
using namespace std;

void greedy_coloring(vector<vector<int>> adj_list,int results[],unordered_map<int,int> um){
    int v=adj_list.size();
    bool available[v];
    for (int cr = 0; cr < v; cr++) 
        available[cr] = false;
    vector<int>::iterator j;
    for(int i=0;i<v;i++){
        if(um[i]==0 || results[i]!=-1) continue;
        for(j=adj_list[i].begin();j!=adj_list[i].end();j++){
            if(results[*j]!=-1)
                available[results[*j]]=true;
        }

        int available_cr;
        for ( available_cr= 0; available_cr < v;available_cr++) 
            if (available[available_cr] == false) 
                break; 
        results[i]=available_cr;
         for (j = adj_list[i].begin(); j != adj_list[i].end(); j++) 
            if (results[*j] != -1) 
                available[results[*j]] = false; 
    }
    return;
}

vector<int> find_conflicts(int results[],int random_weights[],vector<vector<int>> adj_list,int e){
    vector<int> conflicts;
    for(int i=0;i<e;i++){
        vector<int>::iterator j;
        for(j=adj_list[i].begin();j!=adj_list[i].end();j++){
            if(results[i]==results[*j]){
                if(random_weights[i]<random_weights[*j]){
                    results[i]=-1;
                    conflicts.push_back(i);
                    break;
                }
            }
        }
    }
    return conflicts;
}

int main(int argc, char ** argv){
	MPI_Init(&argc, &argv); 
    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    enum role_ranks { SENDER, RECEIVER }; 
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	if(rank==0){
        string input_file=argv[1];
        string output_file=argv[2];
		int v,e;
        ifstream is(input_file);
        ofstream out;
        out.open(output_file);
		is>>v;
        is>>e;
        int edges[v][2];
		for(int i=0;i<e;i++){
            int v1,v2;
            is>>v1;
            is>>v2;
            edges[i][0]=v1;
            edges[i][1]=v2;
        }
        // Construct line graph and do vertex coloring
        vector<vector<int>> adj_list(e);
        int degree[e]={0};
        int max_degree=0;
        for(int i=0;i<e;i++){
            for(int j=i+1;j<e;j++){
                int e1_v1=edges[i][0];
                int e1_v2=edges[i][1];
                int e2_v1=edges[j][0];
                int e2_v2=edges[i][1];
                if(e1_v1==e2_v1 || e1_v1==e2_v2 || e1_v2==e2_v1 || e1_v2==e2_v2){
                    adj_list[i].push_back(j);
                    adj_list[j].push_back(i);
                    degree[i]++;
                    degree[j]++;
                }
            }
        }
        int sum=0;
        for(int i=0;i<e;i++){
            max_degree=max(max_degree,degree[i]);
            sum+=degree[i];
        }
        vector<int> conflicts;
        for(int i=0;i<e;i++) conflicts.push_back(i);
        int results[e];
        for(int i=0;i<e;i++) results[e]=-1;
        while(true){
            int ts=conflicts.size();
            int edges[ts];
            int random_weights[ts];
            for(int i=0;i<ts;i++){
                edges[i]=i;
                random_weights[i]=i;
            }
            unsigned seed = 0;
            shuffle(random_weights, random_weights + ts,default_random_engine(seed));
            shuffle(edges, edges + ts,default_random_engine(seed));
            int start=0;
            int step=ceil((float)ts/(float)size);
            int pos_ind=0;
            vector<vector<int>> partitions;
            for(int i=0;i<size;i++){
                int curr_size=min(step,ts-start);
                if(curr_size<=0){ pos_ind=i-1;break;}
                for(int j=0;j<curr_size;j++){
                    partitions[i][j]=edges[start+j];
                }
                start+=curr_size;
            }
           
            for(int i=1;i<size;i++){
                int s;
                if(i<=pos_ind) s=-1;
                else s=partitions[i].size();
                MPI_Send(&s,1,MPI_INT,i,0,MPI_COMM_WORLD);
                if(s!=-1){
                    MPI_Send(partitions[i],partitions[i].size(),MPI_INT,i,0,MPI_COMM_WORLD);
                    MPI_Send(&sum,1,MPI_INT,i,0,MPI_COMM_WORLD);
                    MPI_Send(&e,1,MPI_INT,i,0,MPI_COMM_WORLD);
                    MPI_Send(degree,e,MPI_INT,i,0,MPI_COMM_WORLD);
                    MPI_Send(adj_list,sum,MPI_INT,i,0,MPI_COMM_WORLD);
                    MPI_Send(results,e,MPI_INT,i,0,MPI_COMM_WORLD);
                }
            }
            unordered_map<int,int> um;
            for(int i=0;i<step;i++) um[partitions[0][i]]++;
            results[partitions[0][0]]=0;
            greedy_coloring(adj_list,results,um);
            for(int i=1;i<size;i++){
                int temp[e];
                MPI_Recv(temp,e,MPI_INT,i,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
                for(int i=0;i<e;i++){
                    if(temp[i]!=-1) results[i]=temp[i];
                }
            }
            conflicts.clear();
            conflicts=find_conflicts(results,random_weights,adj_list,e);
            if(conflicts.size()==0) break;
        }
        for(int i=0;i<e;i++) out<<results[i]<<" ";
        // for(int i=0;i<e;i++){
        //     int s=adj_list[i].size();
        //     cout<<i<<"->";
        //     for(int j=0;j<s;j++) cout<<adj_list[i][j]<<"->";
        //     cout<<endl;
        // }
		// for(int i=0;i<e;i++) max_degree=max(max_degree,degree[i]);
        // cout<<max_degree<<endl;
	}
	else{
		int s;
        MPI_Recv(&s,1,MPI_INT,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        if(s!=-1){
            int vertices[s];
            int sum;
            int e;
            int degree[e];
            int temp[sum];
            int results[e];
            vector<vector<int>> adj_list;
            MPI_Recv(vertices,s,MPI_INT,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            MPI_Recv(&sum,1,MPI_INT,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            MPI_Recv(&e,1,MPI_INT,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            MPI_Recv(degree,e,MPI_INT,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            MPI_Recv(temp,sum,MPI_INT,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            MPI_Recv(results,e,MPI_INT,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            int count=0;
            for(int i=0;i<e;i++){
                for(int j=0;j<degree[e];j++){
                    adj_list[i].push_back(temp[count++]);
                }
            }
            unordered_map<int,int> um;
            for(int i=0;i<s;i++) um[vertices[i]]++;
            results[vertices[0]]=0;
            greedy_coloring(adj_list,results,um);
            MPI_Send(results,e,MPI_INT,0,0,MPI_COMM_WORLD);
        }
	}
	MPI_Finalize();
	return 0;
}
