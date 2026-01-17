#ifndef GRAPH_H
#define GRAPH_H

#include "../boolean.h"
#include "../../utils/user/user.h"

#define MAX_GRAPH_VERTICES 1000

typedef struct {
    int numVertices;
    User *vertices[MAX_GRAPH_VERTICES]; 
    boolean adjMatrix[MAX_GRAPH_VERTICES][MAX_GRAPH_VERTICES];  
    int vertexIndexMap[MAX_GRAPH_VERTICES];  
    int vertexCount;
} Graph;

typedef struct {
    int userIndex;
    int mutualCount;
} FriendRecommendation;

void createGraph(Graph *g);
void addVertex(Graph *g, User *user);
void addEdge(Graph *g, char *follower_id, char *following_id);
void removeEdge(Graph *g, char *follower_id, char *following_id);


boolean hasEdge(Graph *g, char *follower_id, char *following_id);
boolean isVertexInGraph(Graph *g, char *user_id);


int getVertexIndex(Graph *g, char *user_id);
int getInDegree(Graph *g, char *user_id);  
int getOutDegree(Graph *g, char *user_id); 


void findMutualFriends(Graph *g, char *user1_id, char *user2_id, char **mutual, int *count);
void findSuggestedFriends(Graph *g, char *user_id, char **suggested, int *count, int maxSuggestions);
boolean findPath(Graph *g, char *from_id, char *to_id, char **path, int *pathLength);
void findMostPopularUsers(Graph *g, char **popular, int *count, int maxResults);
void findMostActiveUsers(Graph *g, char **active, int *count, int maxResults);


void BFS(Graph *g, char *start_id, boolean *visited);
void DFS(Graph *g, char *start_id, boolean *visited);


boolean isFriend(Graph *g, char *user1_id, char *user2_id);
int countMutual(Graph *g, char *user1_id, char *user2_id);
void getRecommendations(Graph *g, char *user_id, FriendRecommendation *recommendations, int *count, int maxResults);

#endif
