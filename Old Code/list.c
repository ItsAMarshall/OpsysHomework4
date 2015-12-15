/**
 * Lucas Silva
 * silval@rpi.edu
 * Operating Systems HW 3
 * @file The implementation file for a singly linked list data structure.
 *       Used to keep track of files in the server.
 */

#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>

struct file {
    char* name;
    int inUse;
    struct file *next;
}*head, *var, *trav;

void insert_at_begning(int value) {
     var=(struct node *)malloc(sizeof (struct node));
     var->data=value;
     if(head==NULL)
     {
         head=var;
         head->next=NULL;
     }
     else
     {
         var->next=head;
         head=var;
     }
}

void insert_at_end(int value)
{
      struct node *temp;
      temp=head;
      var=(struct node *)malloc(sizeof (struct node));
      var->data=value;
      if(head==NULL)
      {
          head=var;
          head->next=NULL;
      }
      else
      {
          while(temp->next!=NULL)
          {
               temp=temp->next;
          }
          var->next=NULL;
          temp->next=var;
      }
}

void insert_at_middle(int value, int loc)
{
     struct node *var2,*temp;
     var=(struct node *)malloc(sizeof (struct node));
     var->data=value;
     temp=head;

     if(head==NULL)
     {
          head=var;
          head->next=NULL;
     }
     else
     {
          while(temp->data!=loc)
          {
                temp=temp->next;
          }
          var2=temp->next;
          temp->next=var;
          var->next=var2;
     }
}

int delete_from_middle(int value)
{
     struct node *temp,*var;
     temp=head;
     while(temp!=NULL)
     {
          if(temp->data == value)
          {
                if(temp==head)
                {
                     head=temp->next;
                     free(temp);
                     return 0;
                }
                else
                {
                     var->next=temp->next;
                     free(temp);
                     return 0;
                }
          }
          else
          {
               var=temp;
               temp=temp->next;
          }
     }
printf("data deleted from list is %d",value);
}

int delete_from_end()
{
     struct node *temp;
     temp=head;
     while(temp->next != NULL)
     {
          var=temp;
          temp=temp->next;
     }
     if(temp ==head)
     {
          head=temp->next;
          free(temp);
          return 0;
     }
     printf("data deleted from list is %d",temp->data);
     var->next=NULL;
     free(temp);
     return 0;
}

void display()
{
     trav=head;
     if(trav==NULL)
     {
          printf("\nList is Empty");
     }
     else
     {
          printf("\nElements in the List: ");
          while(trav!=NULL)
          {
               printf(" -> %d ",trav->data);
               trav=trav->next;
          }
      printf("\n");
      }
}
