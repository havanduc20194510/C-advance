#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "jrb.h"
#include "dllist.h"

#define INFINITIVE_VALUE  10000000

// truong size - luu tru so bit de bieu dien mot ma
// truong 
typedef struct {
   JRB edges;
   JRB vertices;
} Graph;

Graph createGraph();
void addVertex(Graph graph, int id, int letter);
int getVertex(Graph graph, int id);
void addEdge(Graph graph, int v1, int v2, int weight);
int getEdgeValue(Graph graph, int v1, int v2);
int indegree(Graph graph, int v, int* output);
int outdegree(Graph graph, int v, int* output);
void dropGraph(Graph graph);

typedef struct {
   Graph graph;
   int rootID;
} HuffmanTree;

typedef Dllist PriorityQueue;

typedef struct {
	int size;
	char bits[20]; // maximum 20 bits, it is safe enough
} Coding;

typedef struct {
	int size;
	int * nodes;
} HuffmanTreeArray;

void add2Queue(PriorityQueue q, int graphnode, int frequence);
HuffmanTree makeHuffman(int fTable[]);
void createHuffmanTable(HuffmanTree htree, Coding htable[]);
void setBit(char* huffman, int nbit, int val);
int getBit(char* huffman, int nbit);
void addHuffmanChar(char ch, Coding htable[], char* huffman, int* nbit);
int compress(char* buffer, char*huffman, Coding htable[]);
void writeHuffmanTable(Coding htable[], FILE* f);
void compressFile(char* input, char*output);
void setRoot(int* cursor);
void goLeft(int* cursor);
void goRight(int* cursor);
HuffmanTreeArray makeHuffmanTreeArray(Coding htable[]);
void destroyHuffmanTreeArray(HuffmanTreeArray htree);
int decompress(char * huffman, int nbit, Coding htable[], char * buffer);
void decompressFile(char *input, char *output);

int main()
{
   char buffer[1000] = "fdfdfdfgjkfjkgrutrtrtkgfkgjjfdfsdiruiedfffdffgffg";
   char huffman[1000];
   Coding htable[256];
   int nbit, bytes;
   nbit = compress(buffer, huffman, htable);
   bytes = decompress(huffman, nbit, htable, buffer);
   buffer[bytes] = '\0';
   printf("%s\n", buffer);
   compressFile("hello.txt", "huffman.dat");
   decompressFile("huffman.dat", "hello_new.txt");
}

Graph createGraph()
{
   Graph g; 
   g.edges = make_jrb();  
   g.vertices = make_jrb();  
   return g;
}

void addVertex(Graph g, int id, int letter)
{
     JRB node = jrb_find_int(g.vertices, id);
     if (node==NULL) // only add new vertex 
         jrb_insert_int(g.vertices, id, new_jval_i(letter));            
}

int getVertex(Graph g, int id)
{
     JRB node = jrb_find_int(g.vertices, id);
     if (node==NULL) 
        return 0;
     else                
        return jval_i(node->val);
}     

void addEdge(Graph graph, int v1, int v2, int weight)
{
     JRB node, tree;
     if (getEdgeValue(graph, v1, v2)==INFINITIVE_VALUE)
     {
        node = jrb_find_int(graph.edges, v1);
        if (node==NULL) {
           tree = make_jrb();
           jrb_insert_int(graph.edges, v1, new_jval_v(tree));
        } else
        {
           tree = (JRB) jval_v(node->val);   
        }
        jrb_insert_int(tree, v2, new_jval_i(weight));
     }
}

int getEdgeValue(Graph graph, int v1, int v2)
{
    JRB node, tree;
    node = jrb_find_int(graph.edges, v1);
    if (node==NULL)
       return INFINITIVE_VALUE;
    tree = (JRB) jval_v(node->val);
    node = jrb_find_int(tree, v2);
    if (node==NULL)
       return INFINITIVE_VALUE;
    else
       return jval_i(node->val);       
}

int indegree (Graph graph, int v, int* output)
{
    JRB tree, node;
    int total = 0;   
    jrb_traverse(node, graph.edges)
    {
       tree = (JRB) jval_v(node->val);
       if (jrb_find_int(tree, v))
       {
          output[total] = jval_i(node->key);
          total++;
       }                
    }
    return total;   
}

int outdegree (Graph graph, int v, int* output)
{
    JRB tree, node;
    int total;
    node = jrb_find_int(graph.edges, v);
    if (node==NULL)
       return 0;
    tree = (JRB) jval_v(node->val);
    total = 0;   
    jrb_traverse(node, tree)
    {
       output[total] = jval_i(node->key);
       total++;                
    }
    return total;   
}

void dropGraph(Graph graph)
{
    JRB node, tree;
    jrb_traverse(node, graph.edges)
    {
        tree = (JRB) jval_v(node->val);
        jrb_free_tree(tree);
    }
    jrb_free_tree(graph.edges);
    jrb_free_tree(graph.vertices);
}



void add2Queue(PriorityQueue q, int graphnode, int frequence) {
	Dllist node = dll_first(q);
	while (node!=q) {
		// printf("Node %d %d\n", jval_iarray(dll_val(node))[0], jval_iarray(dll_val(node))[1] );
		if ( frequence <= jval_iarray(dll_val(node))[1] ) break;
		node =  dll_next(node);
	}
	if ( node==q ) {
		// printf("Node nil\n");
		dll_append(q, new_jval_iarray(graphnode, frequence));
	} else {
		// printf("Before %d %d\n", jval_iarray(dll_val(node))[0], jval_iarray(dll_val(node))[1] );
		dll_insert_b(node, new_jval_iarray(graphnode, frequence));	
	}
}

HuffmanTree makeHuffman(int fTable[]) {
	int lastNodeID = 0;
	HuffmanTree	hTree;
	int i;
	Dllist n1, n2;

	PriorityQueue queue = new_dllist();
	hTree.graph = createGraph();

	for (i=0; i<256; i++)
		if (fTable[i]) {
			// make new graphNode to add to the priority queue
			lastNodeID++;
			addVertex(hTree.graph, lastNodeID, i);
			add2Queue(queue, lastNodeID, fTable[i]);
			// printf("last %d %c %d\n", lastNodeID, i, fTable[i]);
		}
    for (;;)
        {
            n1 = dll_first(queue);
            if ( n1==queue ) break;
            n2 = dll_next(n1);
            if ( n2==queue ) break;
            // add new node in the graph
            lastNodeID++;
            addVertex(hTree.graph, lastNodeID, -1);
            addEdge(hTree.graph, lastNodeID, jval_iarray(dll_val(n1))[0], 0); 
            addEdge(hTree.graph, lastNodeID, jval_iarray(dll_val(n2))[0], 1); 
            //printf("last %d\n", lastNodeID);
            // modify the queue
            dll_delete_node(n1);
            dll_delete_node(n2);
            add2Queue(queue, lastNodeID, jval_iarray(dll_val(n1))[1]+jval_iarray(dll_val(n2))[1] );
        }
	n1 = dll_first(queue);
	if ( n1==queue ) 
		hTree.rootID = 0;
	else
		hTree.rootID = jval_iarray(dll_val(n1))[0];	
	
	free_dllist(queue);

	return hTree;
}

void getCode(int node, Graph g, int len, char code[], Coding htable[]) 
{
	int ch, k, i;
	int output[10];
	ch = getVertex(g, node);
	if (ch != -1) {
		htable[ch].size = len;
		memcpy(htable[ch].bits, code, len);	
	} else {
		k = outdegree(g, node, output);
		for (i=0; i<k; i++) {
			code[len] = getEdgeValue(g, node, output[i]);
			getCode(output[i], g, len+1, code, htable);
		}
	}
}

void getCode(int node, Graph g, int len, char code[], Coding htable[]) 
{
	int ch, k, i;
	int output[10];
	ch = getVertex(g, node);
	if (ch != -1) {
		htable[ch].size = len;
		memcpy(htable[ch].bits, code, len);	
	} else {
		k = outdegree(g, node, output);
		for (i=0; i<k; i++) {
			code[len] = getEdgeValue(g, node, output[i]);
			getCode(output[i], g, len+1, code, htable);
		}
	}
}

void createHuffmanTable(HuffmanTree htree, Coding htable[]) {
	int i;
	char code[20];
	for (i=0; i<256; i++) {
		htable[i].size=0;
	}
	if (htree.rootID == 0) return;
	getCode(htree.rootID, htree.graph, 0, code, htable); 
}

void setBit(char* huffman, int nbit, int val) {
	int i, byte, bit;
	byte = nbit/8;
	bit = nbit%8;
	if (val==0) {
		huffman[byte] &= ~(1 << bit);
	} else {
		huffman[byte] |= (1 << bit);
	}
}

int getBit(char* huffman, int nbit) {
	int i, byte, bit;
	byte = nbit/8;
	bit = nbit%8;
	i =	huffman[byte] & (1 << bit);
	return i!=0;
}

void addHuffmanChar(char ch, Coding htable[], char* huffman, int* nbit) {
	int i;
	for (i=0; i<htable[ch].size; i++) {
		setBit(huffman, *nbit, htable[ch].bits[i]);
		(*nbit)++;
	}
}

// return number of bits
int compress(char* buffer, char*huffman, Coding htable[]) {
	HuffmanTree htree;
	int i, k, n, nbit;
	
	int fTable[256] = {} ;
	unsigned int ch;
	
	for (i=0; i<strlen(buffer); i++) {
		ch = buffer[i];
		fTable[ch]++;
	}

	htree = makeHuffman (fTable);
	createHuffmanTable(htree, htable);
	printf("Huffman Table Code:\n");
	for (i=0; i<256; i++) {
		if (htable[i].size > 0) {
			printf ("Char %c ", i);
			for (k=0; k<htable[i].size; k++)
			   printf ("%d", htable[i].bits[k]);
			printf ("\n");   
		}
	}
	
	printf("Original size: %ld bytes\n", strlen(buffer));
	printf("%s\n", buffer);
	// Compress
	n = strlen(buffer);
	nbit = 0;
	for (i=0; i<n; i++)
		addHuffmanChar(buffer[i], htable, huffman, &nbit);
	printf("Compressed size: %d bytes\n", (nbit/8)+1);
	for (i=0; i<nbit; i++)
		printf("%d", getBit(huffman, i));
	printf("\n");
	return nbit;
}