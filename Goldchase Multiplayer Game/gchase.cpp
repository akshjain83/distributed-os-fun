/*
*@File   : gchase.cpp
*@Brief  : This file is main() routine for the Gold Chase Game. 
*          It uses semaphores for the shared memory, which is the 
*          game board accessed by multiple players.
*
*
*@Author : Akshay Jain, <ajain2@mail.csuchico.edu>
*@Credits: Thanks to Todd Gibbson for Screen.*, Map.* and goldchase.h
*@Date   : 02/21/2016
*/


#include <semaphore.h>
#include <iostream>
#include <fstream>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <cstdlib>
#include "Map.h"
#include "goldchase.h"

using namespace std;

//structure for the gameboard/Map.
struct GameBoard
{
   int rows;
   int cols;
   unsigned char player;
   char map[0];
};

int main()
{
   int numberOfGold, height,width, shared_mem_fd, mapArea,b;
   int a=0;
   height =0;
   unsigned char current_player = 0;
   unsigned char playerWOn = 0;
   const char *shmName = "/AJ_shm";
   string lineChar, contents;
   int current_position, cp;
   int i = 0;
   GameBoard *gbp;
   srand(time(NULL));
   sem_t *mySemaphore;
   mySemaphore = sem_open("/AJ4_semaphore", O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1);
   if(mySemaphore != SEM_FAILED)
   {
      current_player |= G_PLR0;
      playerWOn |= G_PLR0;
      ifstream ifs;
      ifs.open("mymap.txt");
      while(!ifs.eof())
      {
         while(getline(ifs,lineChar))
         {
            if(height == 0)
               numberOfGold = atoi(lineChar.c_str());
            else 
            { 
               width = lineChar.length();
               contents += lineChar;
            }
            height++;
         }
         height--;
      }
      ifs.close();
      mapArea = (height*width);
      sem_wait(mySemaphore);
      shared_mem_fd = shm_open(shmName, O_CREAT | O_RDWR, S_IWUSR | S_IRUSR);
      ftruncate(shared_mem_fd, ((sizeof(GameBoard))+mapArea));
      gbp = (GameBoard*)mmap(NULL, ((sizeof(GameBoard))+mapArea), PROT_READ|PROT_WRITE, MAP_SHARED, shared_mem_fd, 0);
      gbp->rows = height;
      gbp->cols = width;
      gbp->player |= current_player;
      while(i<mapArea)
      {
         if(contents[i] ==' ')
         {
            gbp->map[i]=0;
         }   
         else if(contents[i] =='*')
            gbp->map[i]=G_WALL;
         ++i;
      }
      current_position = rand() % mapArea;
      while(gbp->map[current_position]!=0)
      {
         current_position = rand() % mapArea;
      }
      gbp->map[current_position] |= G_GOLD;

      for(int k = 1; k < numberOfGold; k++)
      {
         current_position = rand() % mapArea;
         while(gbp->map[current_position]!=0)
         {
            current_position = rand() % mapArea;
         }
         gbp->map[current_position] |= G_FOOL;
      }
      current_position = rand() % mapArea;
      while(gbp->map[current_position]!=0)
      {
         current_position = rand() % mapArea;
      }
      gbp->map[current_position] |= current_player;
      Map goldMine(gbp->map,gbp->rows,gbp->cols);
      goldMine.postNotice("Welcome to Multplayer Goldchase Arena");
      sem_post(mySemaphore);
      while((a=goldMine.getKey())!='Q')
      {
         if(a=='h')
         {
            if((current_position%width)>0)
            {
               if(!((gbp->map[current_position-1]) & G_WALL))
               {
                  sem_wait(mySemaphore);
                  gbp->map[current_position] &= ~current_player;
                  current_position = current_position - 1;
                  gbp->map[current_position] |= current_player;
                  goldMine.drawMap();
                  sem_post(mySemaphore);
                  if(((gbp->map[current_position]) & G_GOLD) == G_GOLD)
                  {

                     sem_wait(mySemaphore);
                     goldMine.postNotice("You got the real gold! Hurry Up and move towards edge of the Map");
                     gbp->map[current_position] &= ~G_GOLD;
                     sem_post(mySemaphore);
                     playerWOn |= G_GOLD;
                  }
                  else if(((gbp->map[current_position]) & G_FOOL) == G_FOOL)
                  {
                     sem_wait(mySemaphore);
                     goldMine.postNotice("Sorry, You got fooled!");
                     gbp->map[current_position] &= ~G_FOOL;
                     sem_post(mySemaphore);
                  } 
               }
            }
            else if(((G_GOLD & playerWOn) == G_GOLD) && (current_position%width)==0)
            {
               sem_wait(mySemaphore);
               gbp->map[current_position] &= ~current_player;
               gbp->player &= ~current_player;
               goldMine.drawMap();
               goldMine.postNotice("Congratulations, You won the Game!");        
               sem_post(mySemaphore);
               break;
            }
         }
         else if(a=='j')
         {
            if((current_position+width) < mapArea)
            {
               if(!((gbp->map[current_position+width]) & G_WALL))
               {
                  sem_wait(mySemaphore);
                  gbp->map[current_position] &= ~current_player;
                  current_position = current_position + width;
                  gbp->map[current_position] |= current_player;
                  goldMine.drawMap();
                  sem_post(mySemaphore);
                  if(((gbp->map[current_position]) & G_GOLD) == G_GOLD)
                  {

                     sem_wait(mySemaphore);
                     goldMine.postNotice("You got the real gold! Hurry Up and move towards edge of the Map");
                     gbp->map[current_position] &= ~G_GOLD;
                     sem_post(mySemaphore);
                     playerWOn |= G_GOLD;
                  }
                  else if(((gbp->map[current_position]) & G_FOOL) == G_FOOL)
                  {
                     sem_wait(mySemaphore);
                     goldMine.postNotice("Sorry, You got fooled!");
                     gbp->map[current_position] &= ~G_FOOL;
                     sem_post(mySemaphore);
                  } 
               }
            }
            else if((G_GOLD & playerWOn) == G_GOLD)
            {
               sem_wait(mySemaphore);
               gbp->map[current_position] &= ~current_player;
               gbp->player &= ~current_player;
               goldMine.drawMap();
               goldMine.postNotice("Congratulations, You won the Game!");
               sem_post(mySemaphore);        
               break;
            }
         }
         else if(a=='k')
         {
            if(((current_position-width)) > 0)
            {
               if(!((gbp->map[current_position-width]) & G_WALL))
               {
                  sem_wait(mySemaphore);
                  gbp->map[current_position] &= ~current_player;
                  current_position = current_position - width;
                  gbp->map[current_position] |= current_player;
                  goldMine.drawMap();
                  sem_post(mySemaphore);
                  if(((gbp->map[current_position]) & G_GOLD) == G_GOLD)
                  {

                     sem_wait(mySemaphore);
                     goldMine.postNotice("You got the real gold! Hurry Up and move towards edge of the Map");
                     gbp->map[current_position] &= ~G_GOLD;
                     sem_post(mySemaphore);
                     playerWOn |= G_GOLD;
                  }
                  else if(((gbp->map[current_position]) & G_FOOL) == G_FOOL)
                  {
                     sem_wait(mySemaphore);
                     goldMine.postNotice("Sorry, You got fooled!");
                     gbp->map[current_position] &= ~G_FOOL;
                     sem_post(mySemaphore);
                  } 
               }
            }
            else if((G_GOLD & playerWOn) == G_GOLD)
            {
               sem_wait(mySemaphore);
               gbp->map[current_position] &= ~current_player;
               gbp->player &= ~current_player;
               goldMine.drawMap();
               goldMine.postNotice("Congratulations, You won the Game!");  
               sem_post(mySemaphore);      
               break;
            }
         }
         else if(a=='l')
         {

            if(((current_position+1)%width)!=0)
            {
               if(!((gbp->map[current_position+1]) & G_WALL))
               {
                  sem_wait(mySemaphore);
                  gbp->map[current_position] &= ~current_player;//empty loop
                  current_position = current_position + 1;
                  gbp->map[current_position] |= current_player;
                  goldMine.drawMap();
                  sem_post(mySemaphore);
                  if(((gbp->map[current_position]) & G_GOLD) == G_GOLD)
                  {                
                     sem_wait(mySemaphore);
                     goldMine.postNotice("You got the real gold! Hurry Up and move towards edge of the Map");
                     gbp->map[current_position] &= ~G_GOLD;
                     sem_post(mySemaphore);
                     playerWOn |= G_GOLD;
                  }
                  else if(((gbp->map[current_position]) & G_FOOL) == G_FOOL)
                  {
                     sem_wait(mySemaphore);
                     goldMine.postNotice("Sorry, You got fooled!");
                     gbp->map[current_position] &= ~G_FOOL;
                     sem_post(mySemaphore);
                  } 
               }
            }
            else if(((G_GOLD & playerWOn) == G_GOLD) && (((current_position+1)%width)==0)) 
            {
               sem_wait(mySemaphore);
               gbp->map[current_position] &= ~current_player;
               gbp->player &= ~current_player;
               goldMine.drawMap();
               goldMine.postNotice("Congratulations, You won the Game!");    
               sem_post(mySemaphore);    
               break;
            }
         }


      }
      sem_wait(mySemaphore);
      gbp->map[current_position] &= ~current_player;
      gbp->player &= ~current_player;
      sem_post(mySemaphore);
   }
   else
   {
      if(errno==EEXIST)
      {
         current_position = 0;
         current_player = 0;
         mySemaphore = sem_open("/AJ4_semaphore", O_RDWR);
         sem_wait(mySemaphore);
         shared_mem_fd = shm_open(shmName, O_RDWR, S_IWUSR | S_IRUSR);
         int rows,cols;
         read(shared_mem_fd, &rows, sizeof(int));
         read(shared_mem_fd, &cols, sizeof(int));
         gbp = (GameBoard*)mmap(NULL, rows*cols, PROT_READ|PROT_WRITE, MAP_SHARED, shared_mem_fd, 0); 
         sem_post(mySemaphore);
         if(!(gbp->player & G_PLR0))
            current_player = G_PLR0;
         else if(!(gbp->player & G_PLR1))
            current_player = G_PLR1;
         else if(!(gbp->player & G_PLR2))
            current_player = G_PLR2;
         else if(!(gbp->player & G_PLR3))
            current_player = G_PLR3;
         else if(!(gbp->player & G_PLR4))
            current_player = G_PLR4;
         else
         { 
            cout<<"\n\nNO Vacancay...Try again Later! \n";
            return 0;
         }
         sem_wait(mySemaphore);
         Map goldMine(gbp->map,gbp->rows,gbp->cols);
         playerWOn |= current_player;
         gbp->player |= current_player;
         sem_post(mySemaphore);
         current_position = rand() % (rows*cols);
         while(gbp->map[current_position]!=0)
         {
            current_position = rand() % (rows*cols);
         }
         sem_wait(mySemaphore);
         gbp->map[current_position] |= current_player;
         goldMine.drawMap();
         goldMine.postNotice("Welcome to Multplayer Goldchase Arena");
         sem_post(mySemaphore);

         int b=0;
         while((b=goldMine.getKey())!='Q')
         {
            if(b=='h')
            {
               if((current_position%cols)>=0)
               {
                  if(!((gbp->map[current_position-1]) & G_WALL))
                  {
                     sem_wait(mySemaphore);
                     gbp->map[current_position] &= ~current_player;//empty loop
                     current_position = current_position - 1;
                     gbp->map[current_position] |= current_player;
                     goldMine.drawMap();
                     sem_post(mySemaphore);

                     if(((gbp->map[current_position]) & G_GOLD) == G_GOLD)
                     {
                        sem_wait(mySemaphore);                       
                        goldMine.postNotice("You got the real gold! Hurry Up and move towards edge of the Map");
                        gbp->map[current_position] &= ~G_GOLD;
                        sem_post(mySemaphore);            
                        playerWOn |= G_GOLD;
                     }
                     else if(((gbp->map[current_position]) & G_FOOL) == G_FOOL)
                     {
                        sem_wait(mySemaphore); 
                        goldMine.postNotice("Sorry, You got fooled!");
                        gbp->map[current_position] &= ~G_FOOL;
                        sem_post(mySemaphore);
                     } 
                  } 
               }
               else if(((G_GOLD & playerWOn) == G_GOLD) && ((current_position%cols)==0))
               {
                  sem_wait(mySemaphore); 
                  gbp->map[current_position] &= ~current_player;
                  gbp->player &= ~current_player;
                  goldMine.drawMap();
                  goldMine.postNotice("Congratulations, You won the Game!");
                  sem_post(mySemaphore);        
                  break;
               }
            }
            else if(b=='j')
            {
               if((current_position+cols) < (rows*cols))
               {
                  if(!((gbp->map[current_position+cols]) & G_WALL))
                  {
                     sem_wait(mySemaphore); 
                     gbp->map[current_position] &= ~current_player;
                     current_position = current_position + cols;
                     gbp->map[current_position] |= current_player;
                     goldMine.drawMap();
                     sem_post(mySemaphore);        

                     if(((gbp->map[current_position]) & G_GOLD) == G_GOLD)
                     {
                        sem_wait(mySemaphore); 
                        goldMine.postNotice("You got the real gold! Hurry Up and move towards edge of the Map");
                        gbp->map[current_position] &= ~G_GOLD;
                        sem_post(mySemaphore);        
                        playerWOn |= G_GOLD;
                     }
                     else if(((gbp->map[current_position]) & G_FOOL) == G_FOOL)
                     {
                        sem_wait(mySemaphore);             
                        goldMine.postNotice("Sorry, You got fooled!");
                        gbp->map[current_position] &= ~G_FOOL;
                        sem_post(mySemaphore);
                     } 
                  }
               }
               else if((G_GOLD & playerWOn) == G_GOLD)
               {
                  sem_wait(mySemaphore); 
                  gbp->map[current_position] &= ~current_player;
                  gbp->player &= ~current_player;
                  goldMine.drawMap();
                  goldMine.postNotice("Congratulations, You won the Game!"); 
                  sem_post(mySemaphore);
                  break;
               }
            }
            else if(b=='k')
            {
               if(((current_position-cols)) > 0)
               {
                  if(!((gbp->map[current_position-cols]) & G_WALL))
                  {
                     sem_wait(mySemaphore);         
                     gbp->map[current_position] &= ~current_player;
                     current_position = current_position - cols;
                     gbp->map[current_position] |= current_player;
                     goldMine.drawMap();
                     sem_post(mySemaphore);

                     if(((gbp->map[current_position]) & G_GOLD) == G_GOLD)
                     {
                        sem_wait(mySemaphore);                    
                        goldMine.postNotice("You got the real gold! Hurry Up and move towards edge of the Map");
                        gbp->map[current_position] &= ~G_GOLD;
                        playerWOn |= G_GOLD;
                        sem_post(mySemaphore);

                     }
                     else if(((gbp->map[current_position]) & G_FOOL) == G_FOOL)
                     {
                        sem_wait(mySemaphore);                                
                        goldMine.postNotice("Sorry, You got fooled!");
                        gbp->map[current_position] &= ~G_FOOL;
                        sem_post(mySemaphore);

                     } 
                  }
               }
               else if((G_GOLD & playerWOn) == G_GOLD)
               {
                  sem_wait(mySemaphore);                          
                  gbp->map[current_position] &= ~current_player;
                  gbp->player &= ~current_player;
                  goldMine.drawMap();
                  goldMine.postNotice("Congratulations, You won the Game!"); 
                  sem_post(mySemaphore);
                  break;
               }
            }
            else if(b=='l')
            { 
               if(((current_position+1)%cols)!=0)
               {
                  if((!((gbp->map[current_position+1]) & G_WALL)) )
                  {
                     sem_wait(mySemaphore);                                  
                     gbp->map[current_position] &= ~current_player;
                     current_position = current_position + 1;
                     gbp->map[current_position] |= current_player;
                     goldMine.drawMap();
                     sem_post(mySemaphore);
                     if(((gbp->map[current_position]) & G_GOLD) == G_GOLD)
                     {
                        sem_wait(mySemaphore);                         
                        goldMine.postNotice("You got the real gold! Hurry Up and move towards edge of the Map");
                        gbp->map[current_position] &= ~G_GOLD;
                        sem_post(mySemaphore);
                        playerWOn |= G_GOLD;
                     }
                     else if(((gbp->map[current_position]) & G_FOOL) == G_FOOL)
                     {
                        sem_wait(mySemaphore);
                        goldMine.postNotice("Sorry, You got fooled!");
                        gbp->map[current_position] &= ~G_FOOL;
                        sem_post(mySemaphore);

                     } 
                  } 
               }
               else if(((G_GOLD & playerWOn) == G_GOLD) && (((current_position+1)%cols)==0)) 
               {
                  sem_wait(mySemaphore);
                  gbp->map[current_position] &= ~current_player;
                  gbp->player &= ~current_player;
                  goldMine.drawMap();
                  goldMine.postNotice("Congratulations, You won the Game!");
                  sem_post(mySemaphore);        
                  break;
               }
            }

         }
      sem_wait(mySemaphore);
      gbp->map[current_position] &= ~current_player;
      gbp->player &= ~current_player;
      sem_post(mySemaphore);
     }
     else
     {
      perror("Error handling Semaphores : ");
     }
   }
   if(gbp->player == 0)
   {
      shm_unlink("/AJ_shm");
      sem_close(mySemaphore);
      sem_unlink("/AJ4_semaphore"); 
   }    
   return 0; 
}



