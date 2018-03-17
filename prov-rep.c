//parent
  //opens res.txt
  //maps to memory
    //forks child
  //asks if add resources
  //using ME, adds to memory region
  //syncs file
//child
  //every 10 sec, reports:
    //page size using getpagesize()
    //state of resources
    //status of pages in memory with mincore()
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/sem.h>
#include <signal.h>
#define FILENAME "res.txt"
#define START_ADDRESS 0
#define KEY 1409


size_t get_file_length(void){
  struct stat st;
  stat(FILENAME,&st);
  return st.st_size;
}//gets the length of the file in bytes

void map_file(char *p,size_t filesize,int child_pid){
    //size of the file
  int i;

  int sem_fd;
  key_t sem_key;
  int sem_id;
  struct sembuf sop;

  union semun {
    int val;
    struct semid_ds *buf;
    ushort * array;
  } argument;

  sem_fd = KEY;
  sem_id = semget(KEY, 1, 0666 | IPC_CREAT);
  semctl(sem_key,1,SETVAL, argument);
  if (sem_id <0){
    printf("error");
  }
  sop.sem_num = 0;
  sop.sem_op = -1;
  sop.sem_flg = SEM_UNDO;

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
        //kill child process
        kill(child_pid, SIGKILL);
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
      semop(sem_id, &sop, 1);
      p[i]=n_res+'0';
      sop.sem_op = 1;
      semop(sem_id, &sop, 1);
    }
  }//to alter each number
}
void child_process(char *p, size_t filesize){

  int sem_fd;
  key_t sem_key;
  int sem_id;
  struct sembuf sop;

  sem_fd = KEY;
  sem_id = semget(sem_key, 0, IPC_CREAT | 0600);

  //if (sem_id <0){
  //  printf("error");
  //}
  sop.sem_num = 0;
  sop.sem_op = -1;
  sop.sem_flg = SEM_UNDO;



  sleep(10);//sleep 10 seconds
  int size;
  size = getpagesize();
  printf("\n\nsize of page: %d\n", size);
  printf("state of resources: \n");
  semop(sem_id, &sop, 1);
  for (size_t i = 0; i < filesize; i++)//change this
  {
    printf("%c", p[i]);
  }
  unsigned char *vec;
  size = ((filesize+size-1)/size);
  vec = (unsigned char *)malloc(size);
  mincore(START_ADDRESS,filesize,vec);//determince if pages are resident in memory
  //sop.sem_op = 1;
  //semop(sem_id, &sop, 1);
  int i;
  for(i=0;i<size;i++){
    printf("%d", vec[i]);
  }
}

int main()
{
  //semaphore

  //semaphore
  int pid;
  size_t filesize =get_file_length();
  //-----------------------mapping----------------------
  char *p;//mapping of the file, array of ints
  int fd = open(FILENAME, O_RDWR);//open file
  if (fd == -1){
    perror("open file error");
    exit(0);
  }
  p = mmap(START_ADDRESS,filesize,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);//map file
  if (p == MAP_FAILED){perror("mmap");}
  //----------------------------------------------------
  pid = fork();
  //printf("%d",pid);
  if(pid<0){printf("error in child creation");exit(0);}
  if (pid==0){
    //do child process
    while(1){
      child_process(p, filesize);
    }
  }else{
    //do parent process

    map_file(p,filesize,pid);

    munmap(p, filesize);//unmap file
    close(fd);//close file
  }
  return 0;
}
