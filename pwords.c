#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>

typedef struct dict {
  char *word;
  int count;
  struct dict *next;
} dict_t;

char *
make_word( char *word ) {
  return strcpy( malloc( strlen( word )+1 ), word );
}

dict_t *
make_dict(char *word) {
  dict_t *nd = (dict_t *) malloc( sizeof(dict_t) );
  nd->word = make_word( word );
  nd->count = 1;
  nd->next = NULL;
  return nd;
}

dict_t *
insert_word( dict_t *d, char *word ) {
  
  //   Insert word into dict or increment count if already there
  //   return pointer to the updated dict
  
  dict_t *nd;
  dict_t *pd = NULL;		// prior to insertion point 
  dict_t *di = d;		// following insertion point
  // Search down list to find if present or point of insertion
  while(di && ( strcmp(word, di->word ) >= 0) ) { 
    if( strcmp( word, di->word ) == 0 ) { 
      di->count++;		// increment count 
      return d;			// return head 
    }
    pd = di;			// advance ptr pair
    di = di->next;
  }
  nd = make_dict(word);		// not found, make entry 
  nd->next = di;		// entry bigger than word or tail 
  if (pd) {
    pd->next = nd;
    return d;			// insert beond head 
  }
  return nd;
}

void print_dict(dict_t *d) {
  while (d) {
    printf("[%d] %s\n", d->count, d->word);
    d = d->next;
  }
}

int
get_word( char *buf, int n, FILE *infile) {
  int inword = 0;
  int c;  
  while( (c = fgetc(infile)) != EOF ) {
    if (inword && !isalpha(c)) {
      buf[inword] = '\0';	// terminate the word string
      return 1;
    } 
    if (isalpha(c)) {
      buf[inword++] = c;
    }
  }
  return 0;			// no more words
}

#define MAXWORD 1024
void *
words( FILE *infile ) {
  pthread_mutex_t mutex1;
  pthread_mutex_t mutex2;
  dict_t *wd = NULL;
  char wordbuf[MAXWORD]; 
  int x = get_word( wordbuf, MAXWORD, infile);;
  while( x ) {
    pthread_mutex_lock (&mutex1);
    wd = insert_word(wd, wordbuf); // add to dict
    pthread_mutex_unlock (&mutex1);
    pthread_mutex_lock (&mutex2);
    x = get_word( wordbuf, MAXWORD, infile);
    pthread_mutex_unlock (&mutex2);
  }
  pthread_exit(wd);
}

int
main( int argc, char *argv[] ) {
  int NTHREADS = 4;
  pthread_t pth[NTHREADS];
  void *wd;
  void *dc[4];
  int i, j;
  dict_t *d = NULL;
  FILE *infile = stdin;
  if (argc >= 2) {
    infile = fopen (argv[1],"r");
  }
  if( !infile ) {
    printf("Unable to open %s\n",argv[1]);
    exit( EXIT_FAILURE );
  }
  for(i = 0; i<NTHREADS; i++) {
    pthread_create(&pth[i], NULL, words, infile);
    pthread_join(pth[i], &wd);
    dc[i] = wd;

  }
  for(j = 0; j<NTHREADS; j++){
  print_dict( dc[j] );
  }
  fclose( infile );
}


