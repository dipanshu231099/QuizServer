// Server.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>

#define MAXREQ 10000
#define MAXQUEUE 5
#define MAXSTUDENTS 100
#define MAXQUESTION 100
#define MAXQUESTIONTEXT 1000

/* to store the ids of active users */
pthread_mutex_t user_mutex;
char activeUsers[MAXSTUDENTS][200];
int activeUsersFDS[MAXSTUDENTS]={0};
int teamMates[MAXSTUDENTS]={0};
int cntActiveUsers=0;

/* 
question type-1 for threading
*/
char questionText1[MAXQUESTION][MAXQUESTIONTEXT];
char questionAnswer1[MAXQUESTION][MAXQUESTIONTEXT];
char questionExplanation1[MAXQUESTION][MAXQUESTIONTEXT];
/* 
question type-2 for Scheduling
*/
char questionText2[MAXQUESTION][MAXQUESTIONTEXT];
char questionAnswer2[MAXQUESTION][MAXQUESTIONTEXT];
char questionExplanation2[MAXQUESTION][MAXQUESTIONTEXT];
/* 
question type-3 for Memory Management
*/
char questionText3[MAXQUESTION][MAXQUESTIONTEXT];
char questionAnswer3[MAXQUESTION][MAXQUESTIONTEXT];
char questionExplanation3[MAXQUESTION][MAXQUESTIONTEXT];

int noOfQuestionsType1=0;
int noOfQuestionsType2=0;
int noOfQuestionsType3=0;

FILE *questionText1Stream,
    *questionText2Stream,
    *questionText3Stream,
    *questionAnswer1Stream,
    *questionAnswer2Stream,
    *questionAnswer3Stream,
    *questionExplanation1Stream,
    *questionExplanation2Stream,
    *questionExplanation3Stream;

char *line = NULL;
size_t len = 0;
ssize_t read_;

void readFromFile( FILE* stream,char arr[MAXQUESTION][MAXQUESTIONTEXT])
{
    int i=0;
    while ((read_ = getline(&line, &len, stream)) != -1) 
		snprintf(arr[i++],read_,"%s\n",line);
}
void addSampleQuestions()
// sample questions
{
    
    int i=0;
    while ((read_ = getline(&line, &len, questionText1Stream)) != -1) {
		snprintf(questionText1[i++],read_,"%s\n",line);
        noOfQuestionsType1++;
	}
    readFromFile(questionAnswer1Stream,questionAnswer1);
    readFromFile(questionExplanation1Stream,questionExplanation1);
    
    i=0;
    while ((read_ = getline(&line, &len, questionText2Stream)) != -1) {
		snprintf(questionText2[i++],read_,"%s\n",line);
        noOfQuestionsType2++;
	}
    readFromFile(questionAnswer2Stream,questionAnswer2);
    readFromFile(questionExplanation2Stream,questionExplanation2);
    

     i=0;
    while ((read_ = getline(&line, &len, questionText3Stream)) != -1) {
		snprintf(questionText3[i++],read_,"%s\n",line);
        noOfQuestionsType3++;
	}
    readFromFile(questionAnswer3Stream,questionAnswer3);
    readFromFile(questionExplanation3Stream,questionExplanation3);
   
    
}



int joined[MAXSTUDENTS];

int getQuestion(char * qbuf, char* answer,char* explanation,char type){
    // change this line below
    int qno;
    
    if(type=='1')
    {
        
        int tmp =(rand()%(noOfQuestionsType1));
        snprintf(qbuf,MAXREQ, "%s\n",questionText1[tmp]);
        snprintf(answer,MAXREQ, "%s\n",questionAnswer1[tmp]);
        snprintf(explanation,MAXREQ, "%s\n",questionExplanation1[tmp]);
        qno=tmp;
    }
    else if(type=='2')
    {
         
        int tmp =(rand() % (noOfQuestionsType2));
        
        snprintf(qbuf,MAXREQ, "%s\n",questionText2[tmp]);
        snprintf(answer,MAXREQ, "%s\n",questionAnswer2[tmp]);
        snprintf(explanation,MAXREQ, "%s\n",questionExplanation2[tmp]);
        qno=tmp;
    }
    else if(type=='3')
    {
         
        int tmp =(rand() % (noOfQuestionsType3));
        snprintf(qbuf,MAXREQ,"%s\n", questionText3[tmp]);
        snprintf(answer,MAXREQ, "%s\n",questionAnswer3[tmp]);
        snprintf(explanation,MAXREQ,"%s\n", questionExplanation3[tmp]);
        qno=tmp;
    }
    return qno;
}

int checkAnswer(char* a1,char *a2){
    return !strcasecmp(a1,a2+13);
}
void appendToFile(FILE* stream,char arr[MAXQUESTION][MAXQUESTIONTEXT],char nameOfFile[],int qno)
{
    stream = fopen(nameOfFile, "a");
    if (stream == NULL)
    {
        printf("file append error: %s",nameOfFile);
        exit(EXIT_FAILURE);
    }
    /* Write content to file */
    fprintf(stream, "%s", arr[qno]);
    fclose(stream);
}

void* server(void* consockfdarg )
{
    int consockfd = *((int*)consockfdarg);
    char recvbuf[MAXREQ];
    char sendbuf[MAXREQ];
    char team[200];
    int primary=0;
    int teamFD;
    team[0]='$';
    int n;
    int mode=-1;
    int selected=0;
    int isConnected = 0;
    char *sid = malloc(188*sizeof(char));
    char *answer = malloc((MAXREQ-13)*sizeof(char));
    while (1)
    {

        if (!isConnected)
        {
            memset(recvbuf, 0, MAXREQ);
            n = read(consockfd, recvbuf, MAXREQ - 1); /* Recv */
            if (n <= 0)
                break;
            printf("Debug :Recvd id:%s\n", recvbuf);
            isConnected = 1;
            int i=13;
            while(1){
                sid[i-13]=recvbuf[i];
                if(recvbuf[i]=='\0')break;
                i++;
            }
            
        }
        
        // getting mode from user
        if(mode==-1){
            // asking for mode
            memset(sendbuf,0,MAXREQ);
            snprintf(sendbuf, MAXREQ, "Welcome %s for online Quiz on OS\n Select mod :\n Press'I' for individual mode\n Press 'G' for Group mode\n Press 'A' for Admin mode\n", sid );
            write(consockfd, sendbuf, strlen(sendbuf));

            // getting response
            memset(recvbuf, 0, MAXREQ);
            n = read(consockfd, recvbuf, MAXREQ - 1);
            if (n <= 0)
                break;
            printf("Debug :Recvd mode:%s\n", recvbuf);

            //setting value
            if(recvbuf[13]=='A')mode=2;
            else if(recvbuf[13]=='G'){
                mode=1;
            }
            else mode=0;
        }

        /* for indivadual assessment */
        else if(mode==0){
            char answer[MAXREQ];
            char explanation[MAXREQ];

            char question_type='0';
            memset(sendbuf,0,MAXREQ); // asking for type of question
            snprintf(sendbuf,MAXREQ, "Please tell the Question Type:\n '1' for Threads\n '2' for scheduling\n '3' for Memory Management\n");
            write(consockfd, sendbuf, strlen(sendbuf)); // sending to Student
            memset(recvbuf, 0, MAXREQ);
            n = read(consockfd, recvbuf, MAXREQ - 1); // getting response from Admin
            if (n <= 0)
                break;
            printf("Debug :Recvd answer:%s\n", recvbuf);
           
            question_type=recvbuf[13];
            
            if(!(question_type=='1' || question_type=='2' || question_type=='3'))
            question_type ='1';
            
            memset(sendbuf,0,MAXREQ); // getting a question for user and its explanation
            int qno = getQuestion(sendbuf,answer,explanation,question_type);
            write(consockfd, sendbuf, strlen(sendbuf)); // sending to student
            memset(recvbuf, 0, MAXREQ);
            n = read(consockfd, recvbuf, MAXREQ - 1); // getting response from student
            if (n <= 0)
                break;
            printf("Debug :Recvd answer:%s\n", recvbuf);


            memset(sendbuf,0,MAXREQ); // telling student answer right or wrong
            char* repeatQ = "press 'n' for new question, 'q' to quit, 'r' to return to main menu\n";
            if(checkAnswer(answer,recvbuf)){
                snprintf(sendbuf,MAXREQ,"The answer is correct\nSee the following explanation:\n%s\n%s\n",explanation,repeatQ);
                write(consockfd,sendbuf, strlen(sendbuf));
            }
            else {
                snprintf(sendbuf,MAXREQ,"The answer is wrong\nCorrect answer is :%s\nSee the following explanation:\n%s\n%s\n",answer,explanation,repeatQ);
                write(consockfd,sendbuf, strlen(sendbuf));
            }

            memset(recvbuf, 0, MAXREQ);
            n = read(consockfd, recvbuf, MAXREQ - 1); // getting response from new question or not
            if (n <= 0)
                break;
            if(recvbuf[13]=='q')break;
            else if(recvbuf[13]=='r')mode=-1;
            else (recvbuf[13]=='n');

        }

        else if(mode==1){
            // code here for group

            pthread_mutex_lock(&user_mutex);
            for(int i=0;i<MAXSTUDENTS;i++){
                if(activeUsers[i][0]=='$'){
                    snprintf(activeUsers[i],188,"%s",sid);
                    activeUsersFDS[i]=consockfd;
                    cntActiveUsers++;
                    break;
                }
            }
            pthread_mutex_unlock(&user_mutex);

            pthread_mutex_lock(&user_mutex); // showing the active users to user
            memset(sendbuf,0,MAXREQ);
            snprintf(sendbuf, MAXREQ, "Active Users are as follows (Enter the student id you want to collab with):\n" );
            write(consockfd, sendbuf, strlen(sendbuf));
            for(int i=0;i<MAXSTUDENTS;i++){
                memset(sendbuf,0,MAXREQ);
                if(activeUsers[i][0]!='$' && strcmp(activeUsers[i],sid)!=0){
                    snprintf(sendbuf, MAXREQ, "%s\n",activeUsers[i]);
                    write(consockfd, sendbuf, strlen(sendbuf));
                }
            }
            pthread_mutex_unlock(&user_mutex);

            memset(recvbuf, 0, MAXREQ);
            n = read(consockfd, recvbuf, MAXREQ - 1); // getting response for group mate
            if (n <= 0) break;
            printf("Debug :Recvd answer:%s\n", recvbuf);
            
            for(int i=0;i<MAXSTUDENTS;i++){
                if(strcmp(activeUsers[i],recvbuf+13)==0){
                    teamFD=activeUsersFDS[i]; // getting the other's sfd

                    snprintf(team,188,"%s",activeUsers[i]);
                    break;
                }
            }

            memset(sendbuf,0,MAXREQ);
            snprintf(sendbuf, MAXREQ, "Request from %s, do you want to collaborate (y/n)\n",sid ); // asking to other user
            write(teamFD, sendbuf, strlen(sendbuf));

            memset(recvbuf, 0, MAXREQ);
            n = read(teamFD, recvbuf, MAXREQ - 1); // reading from other from y/n
            printf("Debug :Recvd answer:%s\n", recvbuf);

            if(n!=0 && recvbuf[13]=='y'){
                memset(sendbuf,0,MAXREQ);
                snprintf(sendbuf, MAXREQ, "@%s has accepted your request !!\n",team ); // to send message to both users
                write(consockfd, sendbuf, strlen(sendbuf));

                memset(sendbuf,0,MAXREQ);
                snprintf(sendbuf, MAXREQ, "Quiz is going to start. wait for question\nOnly %s can select the type and give the final answer\n",sid ); // to send message to both users
                write(teamFD, sendbuf, strlen(sendbuf));
                write(consockfd, sendbuf, strlen(sendbuf));

                char answer[MAXREQ];
                char explanation[MAXREQ];
                char question_type='1';

                memset(sendbuf,0,MAXREQ); // asking for type of question
                snprintf(sendbuf,MAXREQ, "Please tell the Question Type:\n '1' for Threads\n '2' for scheduling\n '3' for Memory Management\n");
                write(consockfd, sendbuf, strlen(sendbuf));

                memset(recvbuf, 0, MAXREQ);
                n = read(consockfd, recvbuf, MAXREQ - 1); // getting response from primary user
                printf("Debug :Recvd answer:%s\n", recvbuf);
            
                question_type=recvbuf[13];
                
                memset(sendbuf,0,MAXREQ); // getting a question for user and its explanation
                int qno = getQuestion(sendbuf,answer,explanation,question_type);
                write(consockfd, sendbuf, strlen(sendbuf)); // sending to primary
                write(teamFD, sendbuf, strlen(sendbuf)); // sending to other

                while(1){
                    memset(recvbuf, 0, MAXREQ);
                    n = read(consockfd, recvbuf, MAXREQ - 1); // getting conversational message
                    printf("Debug :Recvd answer:%s\n", recvbuf);
                    if(recvbuf[13]=='@'){
                        memset(sendbuf,0,MAXREQ); // getting a question for user and its explanation
                        int tmp=13;
                        for(tmp;tmp<MAXREQ;tmp++){
                            if(recvbuf[tmp]==':')break;
                        }
                        snprintf(sendbuf,MAXREQ,"userIdXXXXXXXXX:from %s:%s",sid,recvbuf+tmp);
                        write(teamFD, sendbuf, strlen(sendbuf)); // sending to primary
                    }
                    else break;
                }

                memset(sendbuf,0,MAXREQ); // telling student answer right or wrong
                if(checkAnswer(answer,recvbuf)){
                    snprintf(sendbuf,MAXREQ,"The answer is correct\nSee the following explanation:\n%s\n",explanation);
                    write(consockfd,sendbuf, strlen(sendbuf));
                    write(teamFD,sendbuf, strlen(sendbuf));
                }
                else {
                    snprintf(sendbuf,MAXREQ,"The answer is wrong\nCorrect answer is :%s\nSee the following explanation:\n%s\n",answer,explanation);
                    write(consockfd,sendbuf, strlen(sendbuf));
                    write(teamFD,sendbuf, strlen(sendbuf));
                }

                mode=-1;
            }

            else {
                memset(sendbuf,0,MAXREQ);
                snprintf(sendbuf, MAXREQ, "Rejected !!" ); // rejecting the primary user
                write(consockfd, sendbuf, strlen(sendbuf));
                teamFD=0;
                team[0]='$';
                mode=-1;
                continue;
            }
            
            memset(sendbuf,0,MAXREQ);
            snprintf(sendbuf, MAXREQ, "Active Users are as follows:\n" );
            write(consockfd, sendbuf, strlen(sendbuf));

            
        }

        else {
            //admin mode

            /*
            asking for type of question
            */
            char question_type='0';
            memset(sendbuf,0,MAXREQ); 
            snprintf(sendbuf,MAXREQ, "Hello Admin! Please tell the Question Type:\n '1' for Threads\n '2' for scheduling\n '3' for Memory Management\n");
            write(consockfd, sendbuf, strlen(sendbuf)); // sending to Admin
            memset(recvbuf, 0, MAXREQ);
            n = read(consockfd, recvbuf, MAXREQ - 1); // getting response from Admin
            if (n <= 0)
                break;
            printf("Debug :Recvd answer:%s\n", recvbuf);
            question_type=recvbuf[13];
            if(!(question_type=='1' || question_type=='2' || question_type=='3'))
            question_type ='1';


            /*
            asking for question text
            */
            memset(sendbuf,0,MAXREQ); 
            snprintf(sendbuf,MAXREQ, "You have selected question Type :%c\n! Please type the question Text..Please be kind.\n",question_type);
            write(consockfd, sendbuf, strlen(sendbuf)); // sending to admin
            memset(recvbuf, 0, MAXREQ);
            n = read(consockfd, recvbuf, MAXREQ - 1); // getting response from admin
            if (n <= 0)
                break;
            printf("Debug :Recvd answer:%s\n", recvbuf);
            if(question_type == '1')
            {
                snprintf(questionText1[noOfQuestionsType1],MAXQUESTIONTEXT,"%s",(recvbuf+13));
                char nameOfFile[26]="questionText1.txt";
                appendToFile(questionText1Stream,questionText1,nameOfFile,noOfQuestionsType1);
                
            }
            else if(question_type == '2')
            {
                snprintf(questionText2[noOfQuestionsType2],MAXQUESTIONTEXT,"%s",(recvbuf+13));
                char nameOfFile[26]="questionText2.txt";
                appendToFile(questionText2Stream,questionText2,nameOfFile,noOfQuestionsType2);
            }
            else
            {
                snprintf(questionText3[noOfQuestionsType3],MAXQUESTIONTEXT,"%s",(recvbuf+13)); 
                char nameOfFile[26]="questionText3.txt";
                appendToFile(questionText3Stream,questionText3,nameOfFile,noOfQuestionsType3);  
            }

            /*
            asking for Answer
            */
            memset(sendbuf,0,MAXREQ); 
            snprintf(sendbuf,MAXREQ, "You have added question Text \n! Please add correct answer\n");
            write(consockfd, sendbuf, strlen(sendbuf)); // sending to admin
            memset(recvbuf, 0, MAXREQ);
            n = read(consockfd, recvbuf, MAXREQ - 1); // getting response from admin
            if (n <= 0)
                break;
            printf("Debug :Recvd answer:%s\n", recvbuf);
            if(question_type == '1')
            {
                snprintf(questionAnswer1[noOfQuestionsType1],MAXQUESTIONTEXT,"%s",(recvbuf+13));
                char nameOfFile[26]="questionAnswer1.txt";
                appendToFile(questionAnswer1Stream,questionAnswer1,nameOfFile,noOfQuestionsType1);
            }
            else if(question_type == '2')
            {
                snprintf(questionAnswer2[noOfQuestionsType2],MAXQUESTIONTEXT,"%s",(recvbuf+13));
                char nameOfFile[26]="questionAnswer2.txt";
                appendToFile(questionAnswer2Stream,questionAnswer2,nameOfFile,noOfQuestionsType2);
            }
            else
            {
                snprintf(questionAnswer3[noOfQuestionsType3],MAXQUESTIONTEXT,"%s",(recvbuf+13)); 
                char nameOfFile[26]="questionAnswer3.txt";
                appendToFile(questionAnswer3Stream,questionAnswer3,nameOfFile,noOfQuestionsType3);  
            }

            /*
            asking for Explantion
            */
            memset(sendbuf,0,MAXREQ); 
            snprintf(sendbuf,MAXREQ, "You have added Solution \n! Please add explanation of above answer\n");
            write(consockfd, sendbuf, strlen(sendbuf)); // sending to admin
            memset(recvbuf, 0, MAXREQ);
            n = read(consockfd, recvbuf, MAXREQ - 1); // getting response from admin
            if (n <= 0)
                break;
            printf("Debug :Recvd answer:%s\n", recvbuf);
            if(question_type == '1')
            {
                snprintf(questionExplanation1[noOfQuestionsType1],MAXQUESTIONTEXT,"%s",(recvbuf+13));
                char nameOfFile[26]="questionExplanation1.txt";
                appendToFile(questionExplanation1Stream,questionExplanation1,nameOfFile,noOfQuestionsType1);
            }
            else if(question_type == '2')
            {
                snprintf(questionExplanation2[noOfQuestionsType2],MAXQUESTIONTEXT,"%s",(recvbuf+13));
                char nameOfFile[26]="questionExplanation2.txt";
                appendToFile(questionExplanation2Stream,questionExplanation2,nameOfFile,noOfQuestionsType2);
            }
            else
            {
                snprintf(questionExplanation3[noOfQuestionsType3],MAXQUESTIONTEXT,"%s",(recvbuf+13));  
                char nameOfFile[26]="questionExplanation3.txt";
                appendToFile(questionExplanation3Stream,questionExplanation3,nameOfFile,noOfQuestionsType3); 
            }
            
            if(question_type == '1')   noOfQuestionsType1++;
            else if(question_type == '2')  noOfQuestionsType2++;
            else noOfQuestionsType3++;  
            
            /*
             ask to quit or continue or to return to main menu
             */
            memset(sendbuf,0,MAXREQ); 
            char* repeatQ = "press 'n' to add new question, 'q' to quit,'r' to return main menu\n";
            snprintf(sendbuf,MAXREQ,"%s",repeatQ);
            write(consockfd,sendbuf, strlen(sendbuf));
            memset(recvbuf, 0, MAXREQ);
            n = read(consockfd, recvbuf, MAXREQ - 1); // getting response from new question or not
            if (n <= 0)
                break;
            if(recvbuf[13]=='q')break;
            else if(recvbuf[13]=='r')mode=-1;
            else (recvbuf[13]=='n');
        }
    }
    pthread_mutex_lock(&user_mutex);
    printf("Closing connection with %s\n",sid);
    for(int i=0;i<MAXSTUDENTS;i++){
        if(strcmp(sid,activeUsers[i])==0){
            activeUsers[i][0]='$';
            cntActiveUsers--;
            break;
        }
    }
    pthread_mutex_unlock(&user_mutex);
    close(consockfd);
}

void openFiles()
{
    questionText1Stream = fopen("questionText1.txt", "r");
	if (questionText1Stream == NULL)
    {
        perror("file error: questionText1.txt");
        exit(EXIT_FAILURE);
    }
    questionAnswer1Stream = fopen("questionAnswer1.txt", "r");
	if (questionAnswer1Stream == NULL)
    {
        perror("file error: questionAnswer1.txt");
        exit(EXIT_FAILURE);
    }
    questionExplanation1Stream = fopen("questionExplanation1.txt", "r");
	if (questionExplanation1Stream == NULL)
    {
        perror("file error: questionExplanation1.txt");
        exit(EXIT_FAILURE);
    }

    questionText2Stream = fopen("questionText2.txt", "r");
	if (questionText2Stream == NULL)
    {
        perror("file error: questionText2.txt");
        exit(EXIT_FAILURE);
    }
    questionAnswer2Stream = fopen("questionAnswer2.txt", "r");
	if (questionAnswer2Stream == NULL)
    {
        perror("file error: questionAnswer2.txt");
        exit(EXIT_FAILURE);
    }
    questionExplanation2Stream = fopen("questionExplanation2.txt", "r");
	if (questionExplanation2Stream == NULL)
    {
        perror("file error: questionExplanation2.txt");
        exit(EXIT_FAILURE);
    }

    questionText3Stream = fopen("questionText3.txt", "r");
	if (questionText3Stream == NULL)
    {
        perror("file error: questionText3.txt");
        exit(EXIT_FAILURE);
    }
    questionAnswer3Stream = fopen("questionAnswer3.txt", "r");
	if (questionAnswer3Stream == NULL)
    {
        perror("file error: questionAnswer3.txt");
        exit(EXIT_FAILURE);
    }
    questionExplanation3Stream = fopen("questionExplanation3.txt", "r");
	if (questionExplanation3Stream == NULL)
    {
        perror("file error: questionExplanation3.txt");
        exit(EXIT_FAILURE);
    }
	
}
void closeFiles()
{
    free(line);
	fclose(questionText1Stream);
    fclose(questionAnswer1Stream);
    fclose(questionExplanation1Stream);
    fclose(questionText2Stream);
    fclose(questionAnswer2Stream);
    fclose(questionExplanation2Stream);
    fclose(questionText3Stream);
    fclose(questionAnswer3Stream);
    fclose(questionExplanation3Stream);
}

void initialiseActiveUsers() {
    for(int i=0;i<MAXSTUDENTS;i++) {
        activeUsers[i][0] = '$';
    }
}

int main()
{
    
	openFiles();
    addSampleQuestions();
    closeFiles();

    initialiseActiveUsers();
    
    pthread_t tid;

    int lstnsockfd, clilen, portno = 49152;
    struct sockaddr_in serv_addr, cli_addr;

    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    // Server protocol
    /* Create Socket to receive requests*/
    lstnsockfd = socket(AF_INET, SOCK_STREAM, 0);

    /* Bind socket to port */
    bind(lstnsockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    printf("Bounded to port\n");
    while (1)
    {
        printf("Listening for incoming connections\n");

        /* Listen for incoming connections */
        listen(lstnsockfd, MAXQUEUE);

        //clilen = sizeof(cl_addr);
        int* consockfd = malloc(sizeof(int));

        /* Accept incoming connection, obtaining a new socket for it */
        *consockfd = accept(lstnsockfd, (struct sockaddr *)&cli_addr,
                           &clilen);

        printf("Accepted connection\n");

        // creating a thread for server
        pthread_create(&tid, NULL, server, (void*)consockfd);
    }
    
    close(lstnsockfd);
}
