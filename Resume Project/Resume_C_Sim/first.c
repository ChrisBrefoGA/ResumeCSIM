#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
//-----------------------------------------------
struct Block// lines per set
{
	unsigned long long int tag;
	int valid;
	int time;
	int Rmiss;
};
//-----------------------------------------------
struct Set// sets in the cache
{
	struct Block* lines;
};
//-----------------------------------------------
struct Set* cache;
int counter = 0;//counter for the times
int reads = 0; 
int writes = 0;
int hits = 0;
int misses = 0;
int writeM = 0;
int readM = 0;//read miss
int Bsize = 0;//block size
int Csize = 0;//cache size
int Assoc = 0;// number of blocks/lines in a set
int Ssize = 0;// # of sets
int replace = 0;// to know if it's lru(2) or fifo(1)
bool assoc = false;// boolean for fully associative
//--------------------------------------------------------------------------------------------------------------------
int getsetSize();//method to get the # of sets in the cache
void BuildCache();// builds the cache based  off the #of sets and associativity
int powerOf(int size);// returns 1 if it's a power of 2, else returns 0
int getBits(int size);// finds the number of bits needed
void delete(int index, unsigned long long int temp);//delete function for the cache
void Read(unsigned long long int tag,unsigned long long int temp,int index, int prefetch);//read function for the cache
void Write(int index, int prefetch, unsigned long long int temp, unsigned long long int tag);//write for function for the cache
void tagBuild(char oper, int prefetch, unsigned long long int tag);//builds the tag to go be read or written
//---------------------------------------------------------------------------------------------------------------------
int main(int argc, char** argv){
if(argc != 6)
{
printf("not enough arguments\n");
return 0;
}
//---------------------------------------------------------------------------------------------------------------------
FILE* Freader;
Bsize = atoi(argv[4]);// Block Size
int b = 0;// for the powerOf method(block size)
b = powerOf(Bsize);
if(b != 1)
{
	printf("sizes are incorrect");
	return 0;
}
Csize = atoi(argv[1]);//Cache size
int c = 0;// for the powerOf method(cache size)
c = powerOf(Csize);
if(c != 1)
{
	printf("sizes are incorrect");
	return 0;
}
char* num;// to get the number if argument is assoc:n
int a = 0;// for the powerOf method(assoc)
//-----------------------------------------------------------------------------------------------------------------------
if(strcmp(argv[2],"direct") == 0)// when it's direct
{
	Assoc = 1;
}
else if(strcmp(argv[2], "assoc") == 0)// when it's fully associative
{
	Assoc = Csize/Bsize;
	assoc = true;
}
else// when it's assoc with a #
{
	num = argv[2];
	memmove(&num[0], &num[0+6], strlen(num));
	Assoc = atoi(num);
}
//-------------------------------------------------------------------------------------------------------------------------
if(strcmp(argv[3],"lru") == 0)// if it's lru replacement
{
	replace = 2;
}
else// if it's fifo replacement
{
	replace = 1;
}//------------------------------------------------------------------------------------------------------------------------
a = powerOf(Assoc);
if(a != 1)
{
printf("sizes are incorrect");
return 0;
}
//--------------------------------------------------------------------------------------------------------------------------
unsigned int Faddress1;// for the pc counter
unsigned int Faddress;// for the reading of the memory address
unsigned long long int address;// to turn the address into a long long int
char colon;// for the colon
char rw;// for the R or W
Freader = fopen(argv[5], "r");
Ssize = getsetSize();
BuildCache();
while(fscanf(Freader,"%x %c %c %x", &Faddress1, &colon, &rw, &Faddress) == 4)//no-prefetch
{
	   address = Faddress;
	   tagBuild(rw, 1, address);
}
fclose(Freader);
printf("no-prefetch\n");
printf("Memory reads: %d\n", reads);
printf("Memory writes: %d\n", writes);
printf("Cache hits: %d\n", hits);
printf("Cache misses: %d\n", misses);
reads = 0; 
writes = 0;
hits = 0;
misses = 0;
counter = 0;
writeM = 0;
readM = 0;
Freader = fopen(argv[5], "r");
cache = NULL;
BuildCache();
while(fscanf(Freader,"%x %c %c %x", &Faddress1, &colon, &rw, &Faddress) == 4)//with-prefetch
{
	   address = Faddress;
	   tagBuild(rw, 2, address);
}
printf("with-prefetch\n");
printf("Memory reads: %d\n", reads);
printf("Memory writes: %d\n", writes);
printf("Cache hits: %d\n", hits);
printf("Cache misses: %d\n", misses);
fclose(Freader);
return 0;
}
//-----------------------------------------------------------------------------------------------------------------------
int getsetSize()
{
	return Csize/(Assoc*Bsize);
}
//------------------------------------------------------------------------------------------------------------------------
void BuildCache()
{
	if(Assoc > 0)
	{
		cache = (struct Set*) malloc(sizeof(struct Set)*Ssize);
		for(int i = 0; i < Ssize; i++)
		{
		cache[i].lines = (struct Block*) malloc(Assoc*sizeof(struct Block));
		}
	}
	else if(assoc == true)
	{
		cache = (struct Set*) malloc(sizeof(struct Set));
		cache[0].lines = (struct Block*) malloc(Assoc*sizeof(struct Block));
	}
}
//-------------------------------------------------------------------------------------------------------------------------
int powerOf(int size)
{
   int temp = size;
   int power = 1;
	while(power < temp)
	{
		power *= 2;
	}
	if(power == temp) return 1;
   	return 0;
}
//-------------------------------------------------------------------------------------------------------------------------
int getBits(int size)
{
   int temp = size;
   int i = 0;
	while(temp != 1)
	{
	 temp = temp/2;
	 i++;
	}
   return i;
}
//-------------------------------------------------------------------------------------------------------------------------
void delete(int index, unsigned long long int temp)
{
	int Rblock = 0;
	int Tsize = cache[index].lines[0].time;
	for(int i = 1; i < Assoc; i++)
	{
	   if(cache[index].lines[i].time < Tsize)
	     {
		Rblock = i;
		Tsize = cache[index].lines[i].time;
	     }
	}

	cache[index].lines[Rblock].tag = temp;
	cache[index].lines[Rblock].Rmiss = 0;
	cache[index].lines[Rblock].time = counter;

	return;
}
//--------------------------------------------------------------------------------------------------------------------------
void Read(unsigned long long int tag, unsigned long long int temp, int index, int prefetch)
{
int count = 0;
	if(prefetch == 1)//no-prefetch
	{
		while(count < Assoc)
	  	{
	   		if(cache[index].lines[count].tag == temp)// read-hit
			{
		  		if(replace == 2)//lru update
		    		{	
				   counter++;	
				   cache[index].lines[count].time = counter;
		    		}	
				hits++;
		  		break;
			}
	   		count++;
		}
		if(count == Assoc) //read-miss
		{ 
			reads++;
			misses++;
			readM = 1;
			tagBuild('W', prefetch, tag);
			return;
		}
	}
     else//with-prefetch
	{
	   while(count < Assoc)
	    {
	      if(cache[index].lines[count].tag == temp) //read-hit
		{
		   if(replace == 2)//lru update
		     {	
			counter++;		
		        cache[index].lines[count].time = counter;
		     }
	      	   hits++;
	  	   break;
	        }
  		   count++;
	    }
	   if(count == Assoc)//read-miss
	     {
		reads++;
		misses++;
		readM = 2;
		tagBuild('W', prefetch, tag);
		readM = 3;
		tagBuild('W', prefetch, (tag+Bsize));
		return;
	      }
	}
return;
}
//------------------------------------------------------------------------------------------------------------------------
void Write(int index, int prefetch, unsigned long long int temp, unsigned long long int tag){
int count = 0;
 if(writeM == 1 && prefetch == 2)//for write-miss with-prefetch
{
while(count < Assoc)
	{
	   if(cache[index].lines[count].tag == temp)//for write-hit on prefetched tag
	     {
	     	writeM = 0;
	    	break;
	     }
	   else if(cache[index].lines[count].valid == 0)//for write-miss on prefetched tag
	     {
		reads++;
		cache[index].lines[count].tag = temp;
		cache[index].lines[count].valid = 1;
		cache[index].lines[count].time = counter;
		writeM = 0;
		break;
	     }
   		count++;
	}
	if(count == Assoc)//for write-deletion on prefetched tag
	{
		reads++;
		writeM = 0;
		delete(index, temp);
		
	}
} 
else if(readM > 0)// for read-miss
{
	if(readM == 1)//for read-miss no-prefetch
	{
		while(count < Assoc)
		{
		    if(cache[index].lines[count].valid == 0)//for read-miss-insertion
		     {
			cache[index].lines[count].tag = temp;
			cache[index].lines[count].valid = 1;
			cache[index].lines[count].time = counter;
			break;
		     }
	   		count++;
		}
		if(count == Assoc)//for read-miss-deletion
		{
			delete(index, temp);
		}
	}
	else if(readM == 2)//for read-miss with-prefetch
	{
		while(count < Assoc)
		{
		    if(cache[index].lines[count].valid == 0)//for read-miss-insertion
		     {
			cache[index].lines[count].tag = temp;
			cache[index].lines[count].valid = 1;
			cache[index].lines[count].time = counter;
			break;
		     }
	   		count++;
		}
		if(count == Assoc)//for read-miss-deletion
		{
			delete(index, temp);
		}
	}
	else if(readM == 3)// for the prefetched tag
	{
		while(count < Assoc)
		{
		   if(cache[index].lines[count].tag == temp)
			{
			    break;
			}
		    if(cache[index].lines[count].valid == 0)//for read-miss-insertion
		     {
			reads++;
			cache[index].lines[count].tag = temp;
			cache[index].lines[count].valid = 1;
			cache[index].lines[count].time = counter;
			break;
		     }
	   		count++;
		}
		if(count == Assoc)//for read-miss-deletion
		{
			reads++;
			delete(index, temp);
		}
	}
	readM = 0;
	return;
}
else
{
	while(count < Assoc)
	{
	   if(cache[index].lines[count].tag == temp)//for write-hit, no reads
	     {
	     	writes++;
	     	hits++;
			if(replace == 2) 
		     {			
		        cache[index].lines[count].time = counter;
		     }
	     	break;
	     }
	   else if(cache[index].lines[count].valid == 0)//for write-miss
	     {
		writes++;
		misses++;
		reads++;
		cache[index].lines[count].tag = temp;
		cache[index].lines[count].valid = 1;
		cache[index].lines[count].time = counter;
		if(prefetch == 2)
		{
			writeM = 1;
			tagBuild('W', prefetch, (tag+Bsize));
		}
		break;
	     }
   		count++;
	}
	if(count == Assoc)//for write-deletion
	{
		reads++;
		misses++;
		writes++;
		delete(index, temp);
		if(prefetch == 2)
		{
			writeM = 1;
			tagBuild('W', prefetch, (tag+Bsize));
		}
	}
}
return;
}
//------------------------------------------------------------------------------------------------------------------------
void tagBuild(char oper, int prefetch, unsigned long long int tag)
{
unsigned long long int temp = tag;// used for read function in case of a hit or miss
int index = 0;//used to shift the tag
int sIndex = 0;//used to find what set an index goes to
int Boffset = 0;//used to shift the tag
Boffset = getBits(Bsize);
temp = temp >> Boffset;
   if(assoc == false)// when it's more than one set
    {
	 index = getBits(Ssize);
  	 sIndex = temp % Ssize; 
  	 temp = temp >> index;
    }
   if(oper == 'R')// for read operation
	{
	   Read(tag, temp, sIndex, prefetch);
	}
   if(oper == 'W')//for write operation
	{
	   counter++;
	   Write(sIndex, prefetch, temp, tag);
	}
return;
}
