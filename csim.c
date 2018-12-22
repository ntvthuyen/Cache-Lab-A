/*
 * Nguyen Truong Vinh Thuyen
 * 1751042
 */

#include "cachelab.h"
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <math.h>
#include <stdlib.h>

typedef unsigned long long Tag;
typedef int Index;
typedef struct{
    int s;
    int b;
    int E;
}CacheProperties;

typedef struct{
    int misses;
    int hits; 
    int evictions;
}Count;

typedef struct{
    int valid;
    Tag tag;
    char * block;
    int lastest;
}Line;

typedef struct{
    Line * line;
}Set;

typedef struct{
    Set * set;
}Cache;

int input(int argc, char ** argv,CacheProperties *properties, char ** trace, int *v);
int isFull(CacheProperties properties, Set set);
Index freeOne(CacheProperties properties, Set set);
Index getMinMax(CacheProperties properties, Set set,  int isGetMax); // -1 Min or 1 Max
Cache init(CacheProperties properties);
void open(CacheProperties *properties, Cache *cache, char * trace, Count *counter, int v);
void process(Cache *cache,CacheProperties *properties, unsigned long long address, Count *counter, int v);
void help();
//void deallocate();
int main(int argc, char ** argv){
	Count counter;
	CacheProperties prop;
	Cache myCache;
	char * trace;
	int v = 0;
	counter.misses = 0;
	counter.hits = 0;
	counter.evictions = 0;
	switch(input(argc,argv,&prop, &trace, &v)){
	case 1: exit(0);
	case -1:exit(1);	
	}
	myCache = init(prop);
	//myCache.set[0].line[0].valid = 0;
	//++counter.misses;
	open(&prop, &myCache, trace, &counter, v);
	//deallocate(myCache, prop);
	printSummary(counter.hits , counter.misses, counter.evictions);
	return 0;
}
int input(int argc, char ** argv, CacheProperties *properties, char ** trace, int *v){
	char input;
	while( (input=getopt(argc,argv,"s:E:b:t:vh")) != -1){
        switch(input){
       	case 's':{
        	properties->s = atoi(optarg);
       		break;
		}
       	case 'E':{
        	properties->E = atoi(optarg);
       	     	break;
		}
       	case 'b':{
        	properties->b = atoi(optarg);
		break;
		}
       	case 't':{
        	*trace = optarg;
		break;
		}
       	case 'v':{
      	 	*v = 1;
      		break;
		}
	case 'h':{
		help();
	    	return 1;
		}
	default:
		help();
            	return -1;
	        }
	}
	return 0;
}

Cache init(CacheProperties properties){
    Cache temp;
    Set set;
    Line line;
    long long int S = pow(2.0,properties.s);
    temp.set = (Set *) malloc(sizeof(Set) * S);
    int i, j;
    for(i = 0;i < S;i++){
        set.line = (Line *) malloc(sizeof(Line) * properties.E);
        temp.set[i] = set;
        for(j = 0 ; j < properties.E; j++){
            line.valid = 0;
            line.tag = 0;
            line.lastest = 0;
            set.line[j] = line;
        }
    }  
    return temp;
}
void open(CacheProperties *properties, Cache *cache, char * trace,  Count *counter, int v){
	FILE * file;
	char operation;
	unsigned long long address;
	int size;
	file = fopen(trace, "r");
    	while(fscanf(file, " %c %llx,%d", &operation, &address, &size) == 3){
        switch(operation){
            case 'I':
                break;
            case 'L':{
		process(cache,properties,address,counter,v);
                break;
		}
            case 'S':{
		process(cache,properties,address,counter,v);
                break;
            	}
            case 'M':{
		process(cache,properties,address,counter,v);
		process(cache,properties,address,counter,v);
                break;
		}
            default:
                break;
        }
}
 	fclose(file);

}
int isFull(CacheProperties properties, Set set){
	int i = 0;
	for(i = 0;i < properties.E;i++){
	        if(!set.line[i].valid) return 1;
    	}
    	return 0;
}
Index freeOne(CacheProperties properties, Set set){
	int i = 0;
	for(i = 0;i < properties.E;i++){
	        if(!set.line[i].valid) return i;
    	}
	return -1;
}
Index getMinMax(CacheProperties properties, Set set, int isGetMax){
	int temp = set.line[0].lastest, i = 0, index = 0;
	for(i = 0;i < properties.E;i++){
		if(set.line[i].lastest * isGetMax > temp * isGetMax){
			index = i;
			temp = set.line[i].lastest;
  	      }
  	  }
	return index;
}
void process(Cache *cache,CacheProperties *properties, unsigned long long address, Count *counter, int v){	
	Tag tag = address >> (properties->s + properties->b);
	//64 - b - s is the size of tag;
	unsigned long long i = (address << (64 - properties->b - properties->s)) >> ((64 - properties->b - properties->s) + properties->b);
	int j = 0, isHit = 0;
	Index temp = 0, max = 0;
	Set set = cache->set[i];
	for (j = 0;j < properties->E;j++){
		Line line = set.line[j];
		if(line.valid && line.tag == tag){ //check if hit
			max = getMinMax(*properties,set,1);
			cache->set[i].line[j].lastest = cache->set[i].line[max].lastest + 1;
			counter->hits++;
			isHit = 1;
			}
    		}
    	if(!isHit && isFull(*properties,set)){
        	temp = freeOne(*properties,set);
		max = getMinMax(*properties,set,1);
		set.line[temp].tag = tag;
        	set.line[temp].valid = 1;
        	cache->set[i].line[temp].lastest = cache->set[i].line[max].lastest + 1;
        	counter->misses++;
    	}else 
	if(!isHit){
		temp = getMinMax(*properties,set,- 1);
		max = getMinMax(*properties,set,1);
       		set.line[temp].tag = tag;
       		cache->set[i].line[temp].lastest = cache->set[i].line[max].lastest + 1;
		counter->misses++;
        	counter->evictions++;
	} 
}
void help(){
	printf("Usage: ./csim-ref [-hv] -s <s> -E <E> -b <b> -t <tracefile>\n");
	printf("-h: Optional help flag that prints usage info\n");
	printf("-v: Optional verbose flag that displays trace info\n");
	printf("-s <s>: Number of set index bits (S= 2sis the number of sets)\n");
	printf("-E <E>: Associativity (number of lines per set)\n"); 
	printf("-b <b>: Number of block bits (B= 2bis the block size)\n"); 
	printf("-t <tracefile>: Name of the valgrindtrace to replay\n");
}
/*
void deallocate(Cache cache, CacheProperties properties){
	int i;
	for(i = 0; i < pow(2.0,properties.E);i++){
		Set set = cache.set[i];
		if(set.line) free(set.line);
	}
}*/
