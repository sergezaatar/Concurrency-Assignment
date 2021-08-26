/*
Name: Jennifer Ulloa
ID: 1001263031

Name: Serge Zaatar Antor
ID: 10016623346
*/

#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>

/*** Constants that define parameters of the simulation ***/

#define MAX_SEATS 3        /* Number of seats in the professor's office */
#define professor_LIMIT 10 /* Number of students the professor can help before he needs a break */
#define MAX_STUDENTS 1000  /* Maximum number of students in the simulation */

#define CLASSA 0
#define CLASSB 1
#define ON_REST 1
#define RESET -1
#define NADA 0

/* TODO */
/* Add your synchronization variables here */
pthread_mutex_t professor_break_mutex;
pthread_mutex_t classa_mutex;
pthread_mutex_t classb_mutex;

sem_t seats;
pthread_cond_t camein;
pthread_cond_t kickedout;

/* Basic information about simulation.  They are printed/checked at the end
* and in assert statements during execution.
*
* You are responsible for maintaining the integrity of these variables in the
* code that you develop.
*/

static int students_in_office;
static int classa_inoffice;
static int classb_inoffice;
static int students_since_break = 0;
static int classtofollow;
static int ainline;
static int binline;
static int profrest;
static int counter;
static int counteratogether;
static int counterbtogether;


typedef struct
{
	int arrival_time;  // time between the arrival of this student and the previous student
	int question_time; // time the student needs to spend with the professor
	int student_id;
	int class;
} student_info;

/* Called at beginning of simulation.
* TODO: Create/initialize all synchronization
* variables and other global variables that you add.
*/
static int initialize(student_info *si, char *filename)
{
	pthread_mutex_init(&professor_break_mutex, NULL);
	pthread_mutex_init(&classa_mutex, NULL);
	pthread_mutex_init(&classb_mutex,NULL);

	sem_init(&seats,0,3);

	students_in_office = 0;
	classa_inoffice = 0;
	classb_inoffice = 0;
	students_since_break = 0;
	classtofollow = -1; //Office is empty
	profrest = 0;
	counter = 0;
	ainline = 0;
	binline = 0;
	counteratogether = 0;
	counterbtogether = 0;

	/* Read in the data file and initialize the student array */
	FILE *fp;

	if((fp=fopen(filename, "r")) == NULL)
	{
		printf("Cannot open input file %s for reading.\n", filename);
		exit(1);
	}

	int i = 0;
	while ( (fscanf(fp, "%d%d%d\n", &(si[i].class), &(si[i].arrival_time),
	&(si[i].question_time))!=EOF) && i < MAX_STUDENTS )
	{
		i++;
	}

	fclose(fp);
	return i;
}

/* Code executed by professor to simulate taking a break
* You do not need to add anything here.
*/
static void take_break()
{
	printf("The professor is taking a break now.\n");
	sleep(5);
	assert( students_in_office == 0 );
	students_since_break = 0;
}

/* Code for the professor thread. This is fully implemented except for synchronization
* with the students.  See the comments within the function for details.
*/
/*
The professor thread is in charge of switching classes if it falls between
the coniditions and it will switch what threads will continue and it is also
incharge of if more than 10 students have entered then the professor will take
its break
*/
void *professorthread(void *junk)
{
	printf("The professor arrived and is starting his office hours\n");

	/* Loop while waiting for students to arrive. */
	while (1)
	{

		/* TODO */
		/* Add code here to handle the student's request.             */
		/* Currently the body of the loop is empty. There's           */
		/* no communication between professor and students, i.e. all  */
		/* students are admitted without regard of the number         */
		/* of available seats, which class a student is in,           */
		/* and whether the professor needs a break. You need to add   */
		/* all of this.                                               */

		if (classtofollow == CLASSB && binline == 0)
		{
			classtofollow = -1;
		}
		else if (classtofollow == CLASSA && ainline == 0)
		{
			classtofollow = -1;
		}
		counter++;
		if (counteratogether == 5 || counterbtogether == 5)
		{
			if (classtofollow == CLASSA)
			{
				classtofollow = CLASSB;
			}
			else
			{
				classtofollow = CLASSA;
			}
			counter++;
		}
		else if (classtofollow == RESET)
		{
			if (classa_inoffice > 0)
			{
				classtofollow = CLASSA;
			}
			else
			{
				classtofollow = CLASSB;
			}
		}

		if (students_since_break == 9)
		{
			profrest = ON_REST;
			pthread_mutex_lock(&professor_break_mutex);
			while(students_in_office != 0)
			{
			}
			take_break();
			pthread_mutex_unlock(&professor_break_mutex);
			profrest--;
		}
	}
	pthread_exit(NULL);
}


/* Code executed by a class A student to enter the office.
* You have to implement this.  Do not delete the assert() statements,
* but feel free to add your own.
*/
/*
	The A student enters and their thread is locked and while it does not fall
	into the given conditions it will unlock the thread to allow another student
	to enter and then lock it again and recheck the conditions
	once it is out of the while then it will run the sem_wait to wait for a new
	student to take a seat and is in class A and then it do the proper increments
	and then it will unlock the threads that came into the class A
*/
void classa_enter()
{

	/* TODO */
	/* Request permission to enter the office.  You might also want to add  */
	/* synchronization for the simulations variables below                  */
	/*  YOUR CODE HERE.                                                     */

	pthread_mutex_lock(&classa_mutex);
	ainline++;
	while(((!((students_in_office < MAX_SEATS && classtofollow == RESET || classtofollow == CLASSA) &&
		classb_inoffice == NADA && counteratogether < 5 && students_since_break < 10 ))))
		{
			pthread_mutex_unlock(&classa_mutex);
                      //  usleep(1000);
			pthread_mutex_lock(&classa_mutex);

		}
	sem_wait(&seats);
	ainline--;
	students_in_office += 1;
	classa_inoffice += 1;
	students_since_break += 1;
	counteratogether += 1;
	counterbtogether = 0;
	pthread_mutex_unlock(&classa_mutex);

}

/* Code executed by a class B student to enter the office.
* You have to implement this.  Do not delete the assert() statements,
* but feel free to add your own.
*/
/*
	The B student enters and their thread is locked and while it does not fall
	into the given conditions it will unlock the thread to allow another student
	to enter and then lock it again and recheck the conditions
	once it is out of the while then it will run the sem_wait to wait for a new
	student to take a seat and is in class B and then it do the proper increments
	and then it will unlock the threads that came into the class B
*/
void classb_enter()
{

	/* TODO */
	/* Request permission to enter the office.  You might also want to add  */
	/* synchronization for the simulations variables below                  */
	/*  YOUR CODE HERE.                                                     */

	pthread_mutex_lock(&classa_mutex);
	binline++;
	while(((!((classtofollow == RESET || classtofollow == CLASSB) && students_in_office < MAX_SEATS &&
		classa_inoffice == NADA && counterbtogether < 5 && students_since_break < 10 ) )))
		{
			pthread_mutex_unlock(&classa_mutex);
                        //usleep(1000);
			pthread_mutex_lock(&classa_mutex);
		}

	sem_wait(&seats);
	binline--;
	students_in_office += 1;
	classb_inoffice += 1;
	counterbtogether += 1;
	students_since_break += 1;
	counteratogether = 0;
	pthread_mutex_unlock(&classa_mutex);

}

/* Code executed by a student to simulate the time he spends in the office asking questions
* You do not need to add anything here.
*/
static void ask_questions(int t)
{
	sleep(t);
}


/* Code executed by a class A student when leaving the office.
* You need to implement this.  Do not delete the assert() statements,
* but feel free to add as many of your own as you like.
*/
/*
This will lock the thread that is leaving and then do the necessary decrements
so that way the person in line gets dequeued and the sem post_post() will signal
the threads that he left and then it will be unlocked
*/
static void classa_leave()
{
	pthread_mutex_lock(&classb_mutex);
	students_in_office -= 1;
	classa_inoffice -= 1;
	sem_post(&seats);
	pthread_mutex_unlock(&classb_mutex);
}

/* Code executed by a class B student when leaving the office.
* You need to implement this.  Do not delete the assert() statements,
* but feel free to add as many of your own as you like.
*/
/*
This will lock the thread that is leaving and then do the necessary decrements
so that way the person in line gets dequeued and the sem post_post() will signal
the threads that he left and then it will be unlocked
*/
static void classb_leave()
{
	pthread_mutex_lock(&classb_mutex);
	students_in_office -= 1;
	classb_inoffice -= 1;
	sem_post(&seats);
	pthread_mutex_unlock(&classb_mutex);

}

/* Main code for class A student threads.
* You do not need to change anything here, but you can add
* debug statements to help you during development/debugging.
*/
void* classa_student(void *si)
{
	student_info *s_info = (student_info*)si;

	/* enter office */
	classa_enter();

	printf("Student %d from class A enters the office\n", s_info->student_id);

	assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
	assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);
	assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
	assert(classb_inoffice == 0 );

	/* ask questions  --- do not make changes to the 3 lines below*/
	printf("Student %d from class A starts asking questions for %d minutes\n", s_info->student_id, s_info->question_time);
	ask_questions(s_info->question_time);
	//printf("%d\n",students_since_break );

	printf("Student %d from class A finishes asking questions and prepares to leave\n", s_info->student_id);

	/* leave office */
	classa_leave();

	printf("Student %d from class A leaves the office\n", s_info->student_id);

	assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
	assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
	assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);

	pthread_exit(NULL);
}

/* Main code for class B student threads.
* You do not need to change anything here, but you can add
* debug statements to help you during development/debugging.
*/
void* classb_student(void *si)
{
	student_info *s_info = (student_info*)si;

	/* enter office */
	classb_enter();

	printf("Student %d from class B enters the office\n", s_info->student_id);

	assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
	assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
	assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);
	assert(classa_inoffice == 0 );

	printf("Student %d from class B starts asking questions for %d minutes\n", s_info->student_id, s_info->question_time);
	ask_questions(s_info->question_time);
	//printf("%d\n",students_since_break );

	printf("Student %d from class B finishes asking questions and prepares to leave\n", s_info->student_id);

	/* leave office */
	classb_leave();

	printf("Student %d from class B leaves the office\n", s_info->student_id);

	assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
	assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
	assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);

	pthread_exit(NULL);
}

/* Main function sets up simulation and prints report
* at the end.
*/
int main(int nargs, char **args)
{
	int i;
	int result;
	int student_type;
	int num_students;
	void *status;
	pthread_t professor_tid;
	pthread_t student_tid[MAX_STUDENTS];
	student_info s_info[MAX_STUDENTS];

	if (nargs != 2)
	{
		printf("Usage: officehour <name of inputfile>\n");
		return EINVAL;
	}

	num_students = initialize(s_info, args[1]);
	if (num_students > MAX_STUDENTS || num_students <= 0)
	{
		printf("Error:  Bad number of student threads. "
		"Maybe there was a problem with your input file?\n");
		return 1;
	}

	printf("Starting officehour simulation with %d students ...\n",
	num_students);

	result = pthread_create(&professor_tid, NULL, professorthread, NULL);

	if (result)
	{
		printf("officehour:  pthread_create failed for professor: %s\n", strerror(result));
		exit(1);
	}

	for (i=0; i < num_students; i++)
	{

		s_info[i].student_id = i;
		sleep(s_info[i].arrival_time);

		//student_type = random() % 2;
		student_type = rand() % 2;

		if (student_type == CLASSA)
		{
			result = pthread_create(&student_tid[i], NULL, classa_student, (void *)&s_info[i]);
		}
		else // student_type == CLASSB
		{
			result = pthread_create(&student_tid[i], NULL, classb_student, (void *)&s_info[i]);
		}

		if (result)
		{
			printf("officehour: thread_fork failed for student %d: %s\n",
			i, strerror(result));
			exit(1);
		}
	}

	/* wait for all student threads to finish */
	for (i = 0; i < num_students; i++)
	{
		pthread_join(student_tid[i], &status);
	}

	/* tell the professor to finish. */
	pthread_cancel(professor_tid);

	printf("Office hour simulation done.\n");

	return 0;
}
