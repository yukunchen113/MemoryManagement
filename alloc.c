//implements resource allocator

//opens res.txt
//maps region with mmap()
//region size is not smaller than file size
//loop
//asks how many units of a resource type is needed
  //subtracts input from resource type if available
//invokes msync() to synchronize the content of mapped file with physical file
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#define FILENAME "res.txt"
size_t get_file_length(void){
  struct stat st;
  stat(FILENAME,&st);
  return st.st_size;
}//gets the length of the file in bytes

void map_file(){
  size_t filesize = get_file_length();
    //size of the file
  char *p;//mapping of the file, array of ints
  int i;
  int fd = open(FILENAME, O_RDWR);//open file
  if (fd == -1){
    perror("open file error");
    exit(0);
  }

  p = mmap(0,filesize,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);//map file
  if (p == MAP_FAILED){perror("mmap");}


  while(1)
  {
    int n_res;//number of resources
    int n_take;//amount of resources taken
    //int x;
    char agree;
    printf("do you want to use a resource [y/n]? ");
    while(1){
      scanf("%c",&agree);
      if(agree=='n'){
        exit(0);
      }
      if(agree=='y'){
        break;
      }
    }

    printf("please name the resource number and amount to take: ");
    scanf("%d%d",&i,&n_take);
    i=i*2;
    n_res = p[i];
    if(n_res<'0'){
      printf("error in retrieving resource");
      exit(0);
    }
    n_res-='0';
    if(n_res<=0){
      printf("resource empty, did not take resourse\n");
    }else if((n_res - n_take)<0){
      printf("resource not enough, current amount available: %d\n", n_res);
    }else{
      n_res-=n_take;
      p[i]=n_res+'0';
    }
  }//to alter each number


  munmap(p, filesize);//unmap file
  close(fd);//close file
}

int main()
{
  map_file();
  return 0;
}
