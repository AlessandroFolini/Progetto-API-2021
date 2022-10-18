// Folini Alessandro - API Project - 2021
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <string.h>

#define INF INT_MAX
#define START 0
#define DIM 3001
#define ASCII_0 48
#define ASCII_9 57
#define SEPARATOR ','
#define LEFT_CHILD (2*i)+1
#define RIGHT_CHILD (2*i)+2

/*
	The following 2 structure types are used to implement the ranking as a max heap, each graph is stored as a variable
	of type node_t where idx contains the index of the graph (0 = first graph read, ecc.) and sum contains the
	lenght of the minimum spanning tree starting from node 0 calculated with Dijkstra's algorithm
*/
typedef struct node {
	int idx;
	unsigned long int sum;
} node_t;

typedef struct Rank{
  node_t *q;
  int len;
  int heapSize;
} Rank_t;

// Custom strtoul and strtok to speed up the input parsing part
unsigned long int myStrtoul(char *str);
char *myStrtok(char *str, char sep);

// Axuxiliary function for the heaps
int returnParent(int i);

// Functions used for the Dijkstra's algorith
unsigned long int dijkstra(unsigned long int *G, int d);

// Functions used for the ranking
void swapNode(node_t *a, node_t *b);
void insert(Rank_t *H, node_t node, int k);
void maxHeapify(Rank_t *H, int i);
void topK(Rank_t *H);

unsigned long int myStrtoul(char *str){
	unsigned long int num = 0;
	if(str == NULL) return 0;
	// checks if the character pointed by str is a digit
	while(*str >= ASCII_0 && *str <= ASCII_9){
		// es. 189 = 1*10*10 + 8*10 + 9
		num = num*10 + *str - ASCII_0;
		str++;
	}
	return num;
}

char *myStrtok(char *str, char sep){
	// static because I need to preserve the value of the variable even when the function returns the token, in
	// order to use it once the function is called again
	static char *s;

	// if str != null it means that it's the first time that the function is called
	if(str != NULL) s = str;
	// if s is an empty string it will have finished to tokenize the input string so it will return NULL
	if(s[0] == '\0') return NULL;

	// on the first time token = str and then in each call it will point to the beginning of whatever remains of the
	// original str after all the previous strtoks
	char *token = s, *i;

	// every time it encounters a ',' it will return the token
	for(i = s; *i != '\0'; i++){
		if(*i == sep){
			*i = '\0';
			s = i + 1;
			if(i == token) token++;
			else return token;
		}
	}

	s = i; // so that at the next call the function knows that there are no other tokens (s = i = '\0')
	return token;
}

int returnParent(int i){
  if(i != 0) return floor((i-1)/2);
  return 0;
}

// Dijkstra's algorithm for the sum of the minimum path from 0 to every other node
// I tried to use a min heap as the priority queue but for some reason it was giving me problems only with some
// graphs of input4, so I changed my mind and decided to try to use only arrays
unsigned long int dijkstra(unsigned long int *G, int d){
	int dist[d], visit[d], count, minDist, next, i;
	unsigned long int sum = 0;

/*
	Set the distance from 0 to each node according to the adjacency matricx
	If G[i][j] == 0 the ditance is INF = INT_MAX, otherwise it's G[i][j]
*/

	for(i = 0; i < d; i++){
		if(G[i] != 0){
			dist[i] = G[i];
		} else {
			dist[i] = INF;
		}
		visit[i] = 0;
	}

	// START is at a 0 distance from itself and I count it as already visited
	visit[START] = 1;
	dist[START] = 0;
	count = 1;

	// I cicle until I visit every node in the graph
	while(count < d-1){ 
		minDist = INF;

		// next is the (not already visited) node at a minimum distance from START
		for(i = 1; i < d; i++){
			if(!visit[i] && (dist[i] < minDist)){
				minDist = dist[i];
				next = i;
			}
		}

		// if the distance of the "closest" node is INF it means that I have no other nodes reachable from START
		if(minDist == INF) break;

		visit[next] = 1;

		// for each not already visited node i I calculate the distance from next to i (if a path exists) 
		// and add it to the distance of next from START (wich is stored in minDist from the previous loop) and if
		// this sum is less then the already existing distance from START to i, I set it as the new distance from
		// START to i
		for(i = 1; i < d; i++){
			if(!visit[i] && (G[next*d + i] != 0) && (minDist + G[next*d + i] < dist[i])){
				dist[i] = minDist + G[next*d + i];
			}
		}
		count++;
	}

	for(i = 1; i < d; i++){
		if(dist[i] != INF) sum += dist[i];
	}

	return sum;
}

// Swaps two graphs in the TopK ranking according to the sum
void swapNode(node_t *a, node_t *b){
  node_t tmp;

  tmp.idx = a->idx;
  tmp.sum = a->sum;

  a->idx = b->idx;
  a->sum = b->sum;

  b->idx = tmp.idx;
  b->sum = tmp.sum;
}

// Inserts a graph in the ranking (used for the first k graphs)
void insert(Rank_t *H, node_t node, int k){
	// If the ranking is full, I insert a new graph only if its sum is less than the maximum sum in the ranking
	// and do a maxHeapify because after the switch the max heap properties could be violated 
	if(H->heapSize == k) {
		if(node.sum < H->q[0].sum){
			H->q[0].idx = node.idx;
			H->q[0].sum = node.sum;
			maxHeapify(H, 0);
		}
	} else { // I insert the first k graphs normally in the max heap
		int i;
		H->q[H->heapSize].idx = node.idx;
		H->q[H->heapSize].sum = node.sum;
		H->heapSize++;
		i = H->heapSize-1;
		// To preserve the max heap properties
		while(i > 0 && (H->q[returnParent(i)].sum < H->q[i].sum)){
			swapNode(&(H->q[returnParent(i)]), &(H->q[i]));
			i = returnParent(i);
		}
	}
}

// Maintains the array structured as a max heap after some changes
void maxHeapify(Rank_t *H, int i){
	int l, r, posmax;
	l = LEFT_CHILD;
	r = RIGHT_CHILD;
	posmax = i;
	if(l < H->heapSize && H->q[l].sum > H->q[i].sum){
		posmax = l;
	}
	if(r < H->heapSize && H->q[r].sum > H->q[posmax].sum){
		posmax = r;
	}
	if(posmax != i){
		swapNode(&(H->q[i]), &(H->q[posmax]));
		maxHeapify(H, posmax);
	}
}

// Prints the idexes of the top k graphs when the command "TopK" is received
void topK(Rank_t *H){
	if(H->heapSize == 0){
		printf("\n");
		return;
	}
	for(int i = 0; i < (H->heapSize)-1; i++){
		printf("%d ", H->q[i].idx);
	}
	printf("%d\n", H->q[(H->heapSize)-1].idx);
}

int main(){
	int i, j, d, k, graphIdx = -1, offset, s;
	/* 
		I initialize the adjacency matricx as an array so in order to access the position G[i][j] I use the offset i*d + j
		from the starting postion of the array
	*/
	unsigned long int *G;
	char buff[DIM], *token;
	char *x;
	node_t n;
	Rank_t *C = malloc(sizeof(Rank_t));

	s = scanf("%d %d \n", &d, &k);

	if(s){} //To avoid the error of not using the scanf returning value

	G = malloc(d * d * sizeof(unsigned long int));

	C->q = malloc(k*sizeof(node_t));
	C->len = k;
	C->heapSize = 0;

	while(fgets(buff, DIM, stdin) != NULL){	// cicles until the end of the file is reached
		if(buff[0] == 'A'){
			// Command received: AggiungiGrafo
			graphIdx++;
			for(i = 0; i < d; i++){
				x = fgets(buff, DIM, stdin);
				if(x){} //To avoid the error of not using the fgets returning value
				token = myStrtok(buff, SEPARATOR);
				for(j = 0; token != NULL; j++){
					offset = i * d + j;
					G[offset] = myStrtoul(token);
					token = myStrtok(NULL, SEPARATOR);
				}
			}
			n.idx = graphIdx;
			n.sum = dijkstra(G, d);

			insert(C, n, k);

		} else if(buff[0] == 'T'){
			// Command received: TopK
			topK(C);
		}
	}

	free(G);
	free(C->q);
	free(C);
	return 0;
}