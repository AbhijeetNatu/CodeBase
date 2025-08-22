#include <stdio.h>
#include <stdlib.h>

#include <windows.h>


struct queue
{
	LARGE_INTEGER data;
	struct queue* next;
};

struct queue* front = NULL;
struct queue* rear = NULL;

//Function Prototypes
void IsEmpty(void);
void Enqueue(LONGLONG);
LARGE_INTEGER Dequeue(void);
void Front(void);
void Rear(void);
void DisplayQueue(void);

extern FILE* gpFile;    


void IsEmpty(void)
{
	if (front == NULL && rear == NULL)
	{
		fprintf(gpFile,"Queue Empty !!\n");


	}
	else
	{
		fprintf(gpFile,"Queue is NOT Empty\n");
	}
}

void Enqueue(LONGLONG element)
{
	LARGE_INTEGER tempElement;
	tempElement.QuadPart = element;

	struct queue* ptr = NULL;
	
	ptr = (struct queue*)malloc(sizeof(struct queue));

	if (ptr == NULL)
	{
		fprintf(gpFile,"Malloc Failed\n");
		return;
	}

	ptr->data = tempElement;
	ptr->next = NULL;

	if (front == NULL && rear == NULL)
	{
		front = ptr;
		rear = ptr;
	}
	else
	{
		rear->next = ptr;
		rear = ptr;
	}
	fprintf(gpFile," %lld element enqueue\n", tempElement.QuadPart);
}

LARGE_INTEGER Dequeue(void)
{
	LARGE_INTEGER retVal;

	retVal.QuadPart = 0;

	struct  queue* temp = front;
	
	if(front==NULL && rear == NULL)
	{
		fprintf(gpFile," No element to delete \n");
	}
	else if (front == rear)
	{
		retVal.QuadPart = temp->data.QuadPart;
		free(temp);
		front = NULL;
		rear = NULL;
	}
	else
	{
		front = front->next;
		retVal.QuadPart = temp->data.QuadPart;
		free(temp);
	}

	return retVal;
}

void DisplayQueue(void)
{
	struct queue* temp = front;

	if (front == NULL && rear == NULL)
	{
		fprintf(gpFile,"Queue is Empty\n");
	}
	else
	{
		fprintf(gpFile,"\n***** QUEUE *****\n");
		while (temp != NULL)
		{
			fprintf(gpFile,"%lld->", temp->data.QuadPart);
			temp = temp->next;
		}
		fprintf(gpFile, "\n");
	}
}
