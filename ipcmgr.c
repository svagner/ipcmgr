#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <errno.h>
#include <err.h>

#define SHMKEYPATH "/dev/null"
//#define SIZEOFSHMSEG	11534336
//#define SIZEOFSHMSEG	115343360000

unsigned int shmsize=11534336;

void usage () {
        fprintf(stderr,"Usage: \n\
For listen and backup data in IPC use:\n\tipcmgr -i <SQ ID> -s <SEGSIZE> [-f <Backup file name>]\n\
For restore data in IPC use:\n\tipcmgr -r -k <SQ key> -s <SEGSIZE> [-f <Backup file name>]\n\
For create data in IPC use:\n\tipcmgr -c -k <SQ Key> -s <SEGSIZE> [-d <Data for write>]\n");
	exit(0);
}

void getsqcontext (int shmid, int save, FILE *f) {
    int rc;  
    void *shm_address;
    size_t count_bytes;

    if ((shm_address = shmat(shmid, NULL, 0))==(void *)(-1))
    {
	perror("main: shmat() failed\n");
	exit(1);
    }
    if (save==0)
	printf("Server Received : \"%s\"\n", (char *)shm_address);
    else {
	count_bytes = fwrite(shm_address, 1, shmsize, f);    
	printf("Writed %lu bytes. fclose(file) %s.\n", (unsigned long)count_bytes, fclose(f) == 0 ? "well done" : "with error");
    };

    rc = shmdt(shm_address);
    if (rc==-1)
	    {
		    perror("main: shmdt() failed\n");
		    exit(1);
	    }
}

void restoresqcontext (int shmkey, FILE *f) {
    int shmid, rc;
    void    *shm_address;
    void    *count_bytes;

    if ((shmid = shmget (shmkey, shmsize, 0666 | IPC_CREAT)) == -1) {
	perror("shmget: shmget failed");
	printf("ErrNum: %d\n",errno);
	printf("ErrNum: %s\n",strerror(errno));
	exit(1);
    } else {
	(void) fprintf(stderr,
	"shmget: shmget returned %d\n", shmid);
    }


    if ((shm_address = shmat(shmid, NULL, 0))==(void *)(-1))
    {
	perror("main: shmat() failed\n");
	exit(1);
    }
//	count_bytes = fwrite(&f, 1, shmsize, shm_address);    
	fread(shm_address, 1, shmsize, f);    
//	printf("Writed %lu bytes. fclose(file) %s.\n", (unsigned long)count_bytes, fclose(f) == 0 ? "well done" : "with error");
    rc = shmdt(shm_address);
    if (rc==-1)
	    {
		    perror("main: shmdt() failed\n");
		    exit(1);
	    }
}

void createsq (int shmkey, char * msg) {
    int shmid, rc;
    void    *shm_address;

    if ((shmid = shmget (shmkey, shmsize, 0666 | IPC_CREAT)) == -1) {
	perror("shmget: shmget failed");
	printf("ErrNum: %d\n",errno);
	printf("ErrNum: %s\n",strerror(errno));
	exit(1);
    } else {
	(void) fprintf(stderr,
	"shmget: shmget returned %d\n", shmid);
    }
    shm_address = shmat(shmid, NULL, 0);
    if ( shm_address==NULL )
    {
	perror("main: shmat() failed");
	exit(1);
    }
    if (msg!=NULL)
	strcpy((char *) shm_address, msg);
/* Detach the shared memory segment from the current process.    */
    rc = shmdt(shm_address);
    if (rc==-1)
    {
	perror("main: shmdt() failed");
	exit(1);
    }

    exit(0);
}
    

int main (int argc, char *argv[]) {
    unsigned int optc=0,sqnum;
    int opt=0, save=0;
    char shmid_char[12];
    char keyvalue[12];
    char bakf_char[1024];
    char msg_create[shmsize];
//    key_t  key;   /* key to be passed to shmget() */
    int  shmflg;   /* shmflg to be passed to shmget() */
    int  shmid=0, create=0, restore=0, rc;   /* return value from shmget() */
    key_t shmkey;
    FILE *bakf;

    while( (opt = getopt(argc,argv,"crk:i:f:d:s:")) != -1) {
	++optc;
	switch(opt) {
	case 'c': create=1;
		  break;
	case 'r': restore=1;
		break;
	case 'd':
		{
		if (create==1 && strlen(keyvalue)>0)
			strcpy(msg_create, optarg);
		}
		  break;
	case 'k': {
			  printf("optarg: %s\n",optarg);
		if ((strlen(optarg) >0 && create==1) || (strlen(optarg) >0 && restore==1))
			strcpy(keyvalue, optarg);
		else
			usage();
		};
		break;
	case 'i': {
		    if (strlen(optarg) >0 ) {
			    strcpy(shmid_char, optarg);
			    shmid = atoi(shmid_char);
		    }
		    else
			    usage();
		    }
		    break;
	case 'f': {
		if (strlen(optarg)>0) 
			strcpy(bakf_char, optarg);
		else
			usage();
		};
		break;
	case 's': {
		if (strlen(optarg)>0) 
			shmsize=atoi(optarg);
		  };
		break;
	default: usage();
		break;
	}
    }
    if (argc<3)
	usage();
    argc -= optind;
    argv += optind;

    if (shmid==0) {
    if (create==1 && strlen(msg_create)>0) {
	sqnum = atoi(keyvalue);    
	createsq(sqnum, msg_create);
    };
    if (create==1 && strlen(msg_create)==0) {
	    sqnum = atoi(keyvalue);    
	    createsq (sqnum, NULL);
    };
    if (create==0 && strlen(msg_create)==0 && restore==0)
	    usage();
    if (restore==1) {
	    sqnum = atoi(keyvalue);
	    if (strlen(bakf_char)>0 && sqnum>0) {
		if ((bakf = fopen(bakf_char, "r"))==NULL) {
		    perror(bakf_char);
		    exit(1);
		}
	    } else
		    usage();
	    restoresqcontext(sqnum,bakf);
    } else
	    usage();
    };

    if (shmid>0) {
	    getsqcontext(shmid,save,NULL);
    };

    if (strlen(bakf_char)>0 && shmid>0) {
	    if ((bakf = fopen(bakf_char, "w+"))==NULL) {
		perror(bakf_char);
		exit(1);
	    }
	    save = 1;
	    getsqcontext(shmid,save,bakf);
    };

    exit(0);
}
