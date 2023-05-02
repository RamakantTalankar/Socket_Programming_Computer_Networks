#include<bits/stdc++.h>
#include<stdio.h>
using namespace std;





void count(int &count_fresh,int &count_rotten,vector<vector<int>>&grid)
    {
         count_fresh=0;
         count_rotten=0;
        int m=grid.size();
        int n=grid[0].size();
        for(int i=0;i<grid.size();i++)
        {
            for(int j=0;j<grid[i].size();j++)
            {
                if(grid[i][j]==1)
                count_fresh++;
                else
                count_rotten++;
            }
        }
    }

    // void manipulate_grid(vector<vector<int>>& grid,int k)
    // {



        
    //     for(int i=0;i<grid.size();i++)
    //     {
    //         for(int j=0;j<grid[0].size();j++)
    //         {

    //             if(grid[i][j]==k)
    //             {
    //                 if(i==0&&j==0)
    //                 {
    //                     if(grid[0][1]==1)
    //                     grid[0][1]=k+1;
    //                     if(grid[1][0]==1)
    //                     grid[1][0]=k+1;
    //                 }
    //                 else if(i==grid.size()-1 && j== grid[0].size()-1)
    //                 {
    //                     if(grid[i][j-1]==1)
    //                         grid[i][j-1]=k+1;
    //                     if(grid[i-1][j]==1)
    //                         grid[i-1][j]=k+1;
    //                 }
    //                 else if(i==grid.size()-1 && j== 0)
    //                 {
    //                     if(grid[i][1]==1)
    //                         grid[i][1]=k+1;
    //                     if(grid[i-1][0]==1)
    //                         grid[i-1][0]=k+1;
    //                 }
    //                 else if(i==0 && j== grid[0].size()-1)
    //                 {
    //                     if(grid[0][j-1]==1)
    //                         grid[0][j-1]=k+1;
    //                     if(grid[1][j]==1)
    //                         grid[1][j]=k+1;
    //                 }
    //                 else if(i==0)
    //                 {
    //                     if(grid[0][j-1]==1)
    //                         grid[0][j-1]=k+1;
    //                     if(grid[0][j+1]==1)
    //                         grid[0][j+1]=k+1;
    //                     if(grid[1][j]==1)
    //                         grid[1][j]=k+1;
    //                 }
    //                 else if(i==grid.size()-1)
    //                 {
    //                      if(grid[i][j-1]==1)
    //                         grid[i][j-1]=k+1;
    //                     if(grid[i][j+1]==1)
    //                         grid[i][j+1]=k+1;
    //                     if(grid[i-1][j]==1)
    //                         grid[i-1][j]=k+1;
    //                 }
    //                 else if(j==0)
    //                 {
    //                     if(grid[i-1][0]==1)
    //                         grid[i-1][0]=k+1;
    //                     if(grid[i+1][0]==1)
    //                         grid[i+1][0]=k+1;
    //                     if(grid[i][j+1]==1)
    //                         grid[i][j+1]=k+1;
    //                 }
    //                 else if(j==grid[0].size()-1)
    //                 {
    //                     if(grid[i-1][j]==1)
    //                         grid[i-1][j]=k+1;
    //                     if(grid[i+1][j]==1)
    //                         grid[i+1][j]=k+1;
    //                     if(grid[i][j-1]==1)
    //                         grid[i][j-1]=k+1;
    //                 }
    //                 else
    //                 {
    //                      if(grid[i][j-1]==1)
    //                         grid[i][j-1]=k+1;
    //                     if(grid[i][j+1]==1)
    //                         grid[i][j+1]=k+1;
    //                     if(grid[i-1][j]==1)
    //                         grid[i-1][j]=k+1;
    //                     if(grid[i+1][j]==1)
    //                         grid[i+1][j]=k+1;
    //                 }
    //             }

    //         }
    //     }

    //     return;
    // }
    //int orangesRotting(vector<vector<int>>& grid) {
        // int count_fresh=0;
        // int count_rotten=0;
        
        // count(count_fresh,count_rotten,grid);
        // int total=count_fresh+count_rotten;
        // if(count_rotten==0)
        // return -1;
        // if(count_fresh==0)
        // return 0;

        
        // int k=2;

        // int t=0;
        
        
        // while(1)
        // {   manipulate_grid(grid,k);
        //     k++;
        //     t++;
        //     count(count_fresh,count_rotten,grid);
        //     if(count_fresh==0)
        //     return t;
        // }
     
        








    //}

    void bfs(int vis[],vector<int> adj[],queue<int> &q,int &count1)
    {
        int x= q.size();

        for(int i=0;i<x;i++)
        {
            int k= q.front();
            q.pop();
            
            for(int j=0;j<adj[k].size();k++)
            {
                if(vis[adj[k][j]]==0)
                {
                    q.push(adj[k][j]);
                    vis[adj[k][j]]=1;
                    count1--;
                }
            }
        }
    }

    int orangesRotting(vector<vector<int>>& grid){


        int count_fresh=0;
        int count_rotten=0;
        
        count(count_fresh,count_rotten,grid);
        int total=count_fresh+count_rotten;
        if(count_rotten==0)
        return -1;
        if(count_fresh==0)
        return 0;

        
        int m=grid.size();
        int n=grid[0].size();
        int V=m*n;

        int vis[V];
        for(int i=0;i<V;i++)
        vis[i]=0;
        vector<int> adj[V];
        for(int k=0;k<V;k++)
        {
            if(k%n!=0)
            {adj[k].push_back(k-1);}
            if(k%n!=n-1)
            {adj[k].push_back(k+1);}
            if(k/n!=0)
            {adj[k].push_back(k-n);}
            if(k/n!=m-1)
            {adj[k].push_back(k+n);}
            
        }

        queue<int> q;
        int count1=0;
        for(int i=0;i<m;i++)
        {
            for(int j=0;j<n;j++)
            {
                if(grid[i][j]==2)
                {
                    q.push(i*n+j);
                    vis[i*n+j]=1;
                }
                if(grid[i][j]==0)
                {
                    vis[i*n+j]=1;
                }
                if(grid[i][j]==1)
                count1++;
            }
        }

    int t=0;
        while(count1>0)
        {
            bfs(vis,adj,q,count1);
            t++;
        }

        return t;


    }


int main()
{
    vector<vector<int>> grid;
    grid[0][0]=2;
    grid[0][1]=1;
    grid[0][2]=1;
    grid[1][0]=1;
    grid[1][1]=1;
    grid[1][2]=0;
    grid[2][0]=0;
    grid[2][1]=1;
    grid[2][2]=1;

    int x=orangesRotting(grid);

    printf("The time taken is %d\n",x);
    


}