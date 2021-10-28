#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>  
#include <string.h>
#include <unistd.h>  //fork
#include <sys/wait.h> //waitpid


struct Patient
{
    char name[50];
    char address[70];
    char TAJ[15];
    char testType[50];
    char express[5];
};

enum MenuState
{
    MAIN,
    COLLECTDATA,
    NEWTEST,
    MODIFYDATA,
    DELETEDATA,
    LISTDATA,
    MORNING_START,
    EXIT
};

void collectTestData(struct Patient *patient, char collectType);
void clearBuffer();
void cutNewLine(char *stringToBeCut);
void writeDataToFile(struct Patient *patient);
enum MenuState newTestWithSameData();
int makeListFromFile(struct Patient *patient, char *typeOfSearch, char needToPrint);
int deleteAndModifyData(struct Patient *patient, char modify);
int modifyData(struct Patient *patient, FILE * writeTo);
void ambulanceSignalHandler(int signumber);
void parentSignalHandler(int signumber);
void endSignalHandler(int signumber);

int main(int argc, char **argv)
{
    int exitApp = 0;
    enum MenuState state = MAIN;
    struct Patient patient;
    printf("Udvozlom az Express Labor alkalmazasaban!\nValasszon az alabbi lehetosegek kozul:\n");

    while(exitApp != 1)
    {
        if(state == MAIN)
        {
            printf("|--- MENU --- |\n[1] Adatfelvetel \n[2] Adatok listazasa \n[3] Adatok torlese \n[4] Adatmodositas \n[5] Reggeli start \n[6] Kilepes \n");
            int c = getchar();
                state = COLLECTDATA;
            if(c == '1')
            else if(c == '2')
                state = LISTDATA;
            else if(c == '3')
                state = DELETEDATA;
            else if(c == '4')
                state = MODIFYDATA;
            else if(c == '5')
                state = MORNING_START;
            else if(c == '6')
                state = EXIT;
            clearBuffer();
        }
        else if(state == COLLECTDATA)
        {
            printf("--- ADATFELVETEL ---\nKerem adja meg a szukseges adatokat! \n");
            collectTestData(&patient, 'f'); //f = full, s = stock
            writeDataToFile(&patient);
            state = newTestWithSameData();
            clearBuffer();
        }
        else if(state == NEWTEST)
        {
            collectTestData(&patient, 's');
            writeDataToFile(&patient);
            state = newTestWithSameData();
            clearBuffer();
        }
        else if(state == LISTDATA)
        {
            printf("--- LISTAZAS ---\n[0] Osszes jelentkezes listazasa\n");
            printf("[1] Listazas COVID-19 teszt szerint\n[2] Tudogyulladas teszt szerint\n[3] Kiutes teszt szerint\n[4] Influenza teszt szerint\n[5] Megfazas teszt szerint\n");
            int c = getchar();
            if(c == '0')
                makeListFromFile(&patient, "all", 'y');
            else if(c == '1')
                makeListFromFile(&patient, "Covid-19", 'y');
            else if(c == '2')
                makeListFromFile(&patient, "Tudogyulladas", 'y');
            else if(c == '3')
                makeListFromFile(&patient, "Kiutes", 'y');
            else if(c == '4')
                makeListFromFile(&patient, "Influenza", 'y');
            else if(c == '5')
                makeListFromFile(&patient, "Megfazas", 'y');
            clearBuffer();
            printf("--- Nyomj ENTERT a fomenuhoz valo visszatereshez ---\n");
            clearBuffer();
            state = MAIN;
        }
        else if(state == DELETEDATA)
        {
            printf("--- ADATOK TORLESE ---\nA torlendo adatokhoz tartozo nev: ");
            fgets(patient.name, sizeof(patient.name), stdin);
            cutNewLine(patient.name);
            printf("%i adatsor lett torolve!\n", deleteAndModifyData(&patient, 'f'));
            state = MAIN;
        }
        else if(state == MODIFYDATA)
        {
            printf("--- ADATOK MODOSITASA ---\nA modositando adatokhoz tartozo nev: ");
            fgets(patient.name, sizeof(patient.name), stdin);
            cutNewLine(patient.name);
            printf("%i adatsor lett frissitve!\n", deleteAndModifyData(&patient, 't'));
            state = MAIN;
        }
        else if(state == MORNING_START)         //ambulance car service
        {
            int status; 
            printf("REGGELI START\n");

            pid_t ambulance1, ambulance2;

            int pipefd1[2];
            if (pipe(pipefd1) == -1){perror("Hiba a pipe nyitasakor!\n"); exit(EXIT_FAILURE);}

            signal(SIGUSR1, ambulanceSignalHandler);
            ambulance1 = fork();
            //AMBULANCE CAR 1 CODE-----------------------------------
            if(ambulance1 == 0)     
            {
                sleep(1);
                kill(getppid(), SIGUSR1);

                signal(SIGUSR1, parentSignalHandler);
                pause();

                char incomingAddresses[100][70];
                char incomingNames[100][50];
                int numberOfToDo = 0;
                close(pipefd1[1]);                                       //Usually we close the unused write end
                read(pipefd1[0], incomingAddresses,sizeof(incomingAddresses));                       // reading 
                read(pipefd1[0], incomingNames, sizeof(incomingNames));
                read(pipefd1[0], &numberOfToDo, sizeof(numberOfToDo));
                if(numberOfToDo > 3)                                    //for example if it is 5 then then ambulance car will write all the 5 address, but we need only 3 because of the ambulance car /day limit
                    numberOfToDo = 3;

                int i;
                printf("Mentoauto 1 olvasta az igenyeket: \n");
                for(i = 0; i < numberOfToDo; ++i)                                                                               
                        printf("  %i. CIM: %s | NEV: %s\n", i+1, incomingAddresses[i], incomingNames[i]);        //print out incoming data            


                printf("Mentoauto 1 uton...\n");
                sleep(1);
                
                kill(getppid(), SIGUSR2);

                exit(0);
            }
            //--------------------------------------------------------
            else
            {
                printf("Mentoauto 1: ");
                pause();
                
                ambulance2 = fork();
                //AMBULANCE CAR 2 CODE--------------------------------
                if(ambulance2 == 0)    
                {
                    sleep(1);
                    kill(getppid(), SIGUSR1);

                    signal(SIGUSR1, parentSignalHandler);
                    pause();
                    
                    waitpid(ambulance1, &status, 0);     
                    char incomingAddresses[100][70];
                    char incomingNames[100][50];
                    int numberOfToDo = 0;
                    close(pipefd1[1]);                                                                   //close the unused write end
                    read(pipefd1[0], incomingAddresses,sizeof(incomingAddresses));                       // reading max sizeof incomingData
                    read(pipefd1[0], incomingNames, sizeof(incomingNames));
                    read(pipefd1[0], &numberOfToDo, sizeof(numberOfToDo));
                    close(pipefd1[0]);        
                    printf("Mentoauto 2 olvasta az igenyeket: \n");                                                 // finally we close the used read end
                    if(numberOfToDo > 6)
                        numberOfToDo = 6;
                    int i;                                        
                    for(i = 3; i < numberOfToDo; ++i)                                                                               
                        printf("  %i. CIM: %s | NEV: %s\n", i+1, incomingAddresses[i], incomingNames[i]);        //print out incoming data                                                     

                    printf("Mentoauto 2 uton...\n");
                    sleep(1);

                    kill(getppid(), SIGUSR2);

                    exit(0);
                } 
                //----------------------------------------------------
                //PARENT CODE-----------------------------------------
                else                    
                {
                    printf("Mentoauto 2: ");
                    pause();
                    //SAVE ALL ADDRESSES from file
                    char addresses[100][70];
                    char names[100][50];
                    int distinctAddressCounter = 0;
                    int numberOfAddresses = 0;
                    FILE * f = fopen("applications", "r");
                    if (f == NULL) 
                    {
                        printf("Nincsenek es nem is voltak jelentkezo paciensek!\n");  
                        exit(1); 
                    }
                    else
                    {
                        char line[200];
                        char *token;
                        while(fgets(line, sizeof(line), f))             
                        {
                            token = strtok(line, ":");
                            strcpy(names[numberOfAddresses], token);
                            token = strtok(NULL, ":");
                            strcpy(addresses[numberOfAddresses], token);
                            ++numberOfAddresses;
                            continue;
                        } 
                        
                    }

                    //COUNT DISTINCT addresses
                    int i, j;
                    j = 0;
                    char distinctAddresses[100][70];
                    char distinctNames[100][50];
                    distinctAddressCounter = numberOfAddresses;
                    for(i = 0; i < numberOfAddresses; ++i)
                    {
                        if(strcmp(addresses[i], addresses[i+1]) == 0)
                        {
                            distinctAddressCounter--;
                        }
                        else
                        {
                            strcpy(distinctAddresses[j], addresses[i]);
                            strcpy(distinctNames[j], names[i]);
                            ++j;
                        }
                    }

                    sleep(1);                                                           //WRITE FOR AMBULANCE 1
                    close(pipefd1[0]);                                                  //Usually we close unused read end
                    write(pipefd1[1], distinctAddresses, sizeof(distinctAddresses));    //write distinct addresses
                    write(pipefd1[1], distinctNames, sizeof(distinctNames));            //write distinct names
                    write(pipefd1[1], &distinctAddressCounter, sizeof(distinctAddressCounter));     //write distinct address counter

                    write(pipefd1[1], distinctAddresses, sizeof(distinctAddresses));        //WRITE FOR AMBULANCE 2
                    write(pipefd1[1], distinctNames, sizeof(distinctNames));
                    write(pipefd1[1], &distinctAddressCounter, sizeof(distinctAddressCounter));
                    close(pipefd1[1]);                                                  // Closing write descriptor 
                    fflush(NULL); 	                                                    // flushes all write buffers (not necessary)  

                    signal(SIGUSR2, endSignalHandler);   
                    if(distinctAddressCounter == 0)
                    {
                        kill(ambulance1, SIGKILL);                                      //we dont have to use ambulance cars
                        kill(ambulance2, SIGKILL);
                        printf("Mara nincs teljesitendo feladat!\n");
                    }     
                    else if(distinctAddressCounter > 0 && distinctAddressCounter <= 3)
                    {
                        kill(ambulance1, SIGUSR1);                                      //we have to use 1 ambulance car, because 1 can handle 3 distinct addresses / day
                        kill(ambulance2, SIGKILL);                                      //we dont have to use the second ambulance car because 1 can handle it
                        printf("Van eleg kapacitas 1 db mentoautoval (%i db)!\n", distinctAddressCounter);
                        pause();
                    }
                    else if(distinctAddressCounter > 3 && distinctAddressCounter <= 6)
                    {
                        kill(ambulance1, SIGUSR1);                                      //we have to use 2 ambulance car, because 1 can handle 3 distinct addresses / day
                        kill(ambulance2, SIGUSR1);
                        printf("Van eleg kapacitas 2 db mentoautoval (%i db)!\n", distinctAddressCounter);
                        pause();
                        pause();
                    }
                    else if(distinctAddressCounter > 6)
                    {
                        kill(ambulance1, SIGUSR1);
                        kill(ambulance2, SIGUSR1);
                        printf("Nincs eleg kapacitas, de elinditunk ketto mentoautot, a tobbi holnapra marad (%i db)!\n", distinctAddressCounter);
                        pause();
                        pause();
                        distinctAddressCounter = 6;                 //if more than 6 address / day, we really need to complete 6 addresses, and ignore the other
                    }

                    waitpid(ambulance1, &status, 0);
                    waitpid(ambulance2, &status, 0);

                    int n;
                    for(n = 0; n < distinctAddressCounter; ++n)     
                    {
                        strcpy(patient.name, distinctNames[n]);
                        deleteAndModifyData(&patient, 'f');  
                    }
                    
                    exitApp = 1;
                }
                //-----------------------------------------------------
                
            }
            
            
        }  
        else if(state == EXIT)
            exitApp = 1;        
    }
    printf("Viszontlatasra!\n");
    return 0;
}

void collectTestData(struct Patient *patient, char collectType)                     //collect the user data
{
    if(collectType == 'f')                                                          //needed for the full new user data
    {                                                                               //and skips it when you just want to make a new record with the same data
        printf("Nev: ");
        fgets(patient->name, sizeof(patient->name), stdin);
        printf("Cim: ");
        fgets(patient->address, sizeof(patient->address), stdin);
        printf("TAJ: ");
        fgets(patient->TAJ, sizeof(patient->TAJ), stdin);
    }
    printf("Vizsgalat tipusa:\n [1] Covid-19 teszt\n [2] Tudogyulladas teszt\n [3] Kiutes\n [4] Influenza\n [5] Megfazas\n");
    int c = getchar();
    if(c == '1')
        strcpy(patient->testType, "Covid-19");
    else if(c == '2')
        strcpy(patient->testType, "Tudogyulladas");
    else if(c == '3')
        strcpy(patient->testType, "Kiutes");
    else if(c == '4')
        strcpy(patient->testType, "Influenza");
    else if(c == '5')
        strcpy(patient->testType, "Megfazas");
    else
        strcpy(patient->testType, "ERVENYTELEN");
    clearBuffer();
    printf("Igenyli-e az EXPRESS szolgaltatast?\n [1] IGEN\n [2] NEM\n");
    c = getchar();
    if(c == '1')
        strcpy(patient->express, "Igen");
    else if(c == '2')
        strcpy(patient->express, "Nem");
    else
        strcpy(patient->express, "?");
    clearBuffer();
}

void clearBuffer()                                      //clears the input buffer from '\n' and unnecessary leftovers
{
    int ch;
    while(((ch = getchar()) !='\n') && (ch != EOF));
}

void cutNewLine(char *stringToBeCut)                    //the fgets stores the '\n' character (gets() not stores it, but fgets() protect you from bufferoverflow error)
{
    int len = strlen(stringToBeCut);
    if( stringToBeCut[len-1] == '\n' )
        stringToBeCut[len-1] = '\0' ;
}

void writeDataToFile(struct Patient *patient)           //write new data to file
{
    char *data[5] = {patient->name, patient->address, patient->TAJ, patient->testType, patient->express};
    int i;                      
    for(i=0; i<5; i++)
        cutNewLine(data[i]);

    FILE * f = fopen("applications", "a");
    if (f == NULL) 
    {
        perror("Error at opening the file\n");   
        exit(-1);      
    }
    fprintf(f, "%s:%s:%s:%s:%s\n", data[0], data[1], data[2], data[3], data[4]);
    fclose(f);
}

enum MenuState newTestWithSameData()                    //you can make new records with same data except for the test type
{
    printf("Szeretne egy masik mintavetelt is igenyelni az elobb megadott adatokkal? [I/N]: ");
    int c = getchar();
    if(c == 'I')
        return NEWTEST;
    else
        return MAIN;
}

int makeListFromFile(struct Patient *patient, char *typeOfSearch, char needToPrint)      //makes list from the datafile
{
    int counter = 0;
    FILE * f = fopen("applications", "r");
    if (f == NULL) 
    {
        printf("Nincsenek es nem is voltak jelentkezo paciensek!\n");   
        exit(1);
    }
    else
    {
        char *data[5] = {patient->name, patient->address, patient->TAJ, patient->testType, patient->express};
        char line[200];

        while(fgets(line, sizeof(line), f))             //read line by line
        {
            cutNewLine(line);                           //cuts the '\n' character from the line
            char *token = strtok(line, ":");            //make tokens from the data
            int i = 0;
            while(token != NULL)                
            {
                strcpy(data[i], token);
                token = strtok(NULL, ":");
                ++i;
            } 

            if(strcmp(typeOfSearch, "all") == 0)        //print out all the data
            {   
                if(needToPrint == 'y')
                    printf("Nev: %s\n Cim: %s\n TAJ: %s\n Vizsgalat tipus: %s\n Express szolgaltatas: %s\n", data[0], data[1], data[2], data[3], data[4]);
                ++counter;
            }  
            else if(strcmp(typeOfSearch, data[3]) == 0) //print out the data with the specified test type
            {
                if(needToPrint == 'y')
                    printf("Nev: %s\n Cim: %s\n TAJ: %s\n Vizsgalat tipus: %s\n Express szolgaltatas: %s\n", data[0], data[1], data[2], data[3], data[4]);     
                ++counter;
            }
        }
        if(needToPrint == 'y')
        {
            if(counter > 0)
                printf("Talalatok szama: %i\n", counter);
            else
                printf("Erre a tipusu vizsgalatra nincs jelentkezes!\n");
        }        
    }   
    fclose(f);
    return counter;
}

int deleteAndModifyData(struct Patient *patient, char modify)       //delete the line(s) from the datafile where the patient name is equal to the user input
{
    char searchedName[70];
    strcpy(searchedName, patient->name);
    int counter = 0;                                
    int tempcounter = 0;
    FILE * stockFile = fopen("applications", "r");                  //open the original file
    if (stockFile == NULL) 
    {
        perror("Error at opening the file\n");   
        exit(-1);      
    }

    FILE * newFile = fopen("temp", "w");                            //open a temporal new file
    if (newFile == NULL) 
    {
        perror("Error at opening the file\n");   
        fclose(stockFile);
        exit(-1);      
    }

    char *data[5] = {patient->name, patient->address, patient->TAJ, patient->testType, patient->express};
    char line[200];
    
    while(fgets(line, sizeof(line), stockFile))                 //read line by line the original file
    {
        char *token = strtok(line, ":");
        tempcounter = counter;
        if(strcmp(data[0], token) == 0 && strlen(data[0]) == strlen(token))         //compare the line first element (name) with the given patient name
        {
            
            int i = 0;                                                                          //if it equals, it stores the values (needed for the modification)
            while(token != NULL)
            {
                strcpy(data[i], token);
                token = strtok(NULL, ":");
                ++i;
            } 
            counter++;                                                                          //it counts the deleted lines
        }
        else
        {
            fprintf(newFile, "%s", token);                                                      //prints the first element (name) to the new file
            while(token != NULL)
            {
                token = strtok(NULL, ":");                              
                if(token != NULL)   
                    fprintf(newFile, ":%s", token);                                             //prints the other data to the new file
            }   
        }

        if(modify == 't' && counter > tempcounter)                                              //if modification is selected, and the first element (name) is equals with the given, 
        {                                                                                       //it calls the modify data function, and passes the new file pointer.
            modifyData(patient, newFile);                                                       //The old line will be deleted, and the new modificated will be written to the new file
            tempcounter++;                                                                      //It will go throuhgt the same-name lines, and you can change the data one-by-one
            strcpy(patient->name, searchedName);      //to find the other names                 
        }                                                                                       
    }
    
    fclose(stockFile);                                                                          //close old file
    fclose(newFile);                                                                            //close new file
    remove("applications");                                                                     //delete old file
    rename("temp", "applications");                                                             //rename new file like the old one

    if(modify == 't')
        return tempcounter;

    return counter;                                                                             //return with the number of deleted lines
}

int modifyData(struct Patient *patient, FILE * writeTo)             //deleteAndModifydata calls it when it finds a to get the new updated data
{
    printf("\nTalalt adatok a megadott nevvel:\nNev: %s\n Cim: %s\n TAJ: %s\n Vizsgalat tipus: %s\n Express szolgaltatas: %s\n", patient->name, patient->address, patient->TAJ, patient->testType, patient->express);
    printf("Modositando adat:\n [1] Nev\n [2] Cim\n [3] TAJ\n [4] Vizsgalat tipus\n [5] Express szolgaltatas igenyles\n");
    int c = getchar();
    clearBuffer();
    if(c == '1')                                                    //get the new data
    {
        printf("Uj nev: ");
        fgets(patient->name, sizeof(patient->name), stdin);
    }
    else if(c == '2')
    {
        printf("Uj cim: ");
        fgets(patient->address, sizeof(patient->address), stdin);
    }
    else if(c == '3')
    {
        printf("TAJ: ");
        fgets(patient->TAJ, sizeof(patient->TAJ), stdin);
    }            
    else if(c == '4')
    {
        printf("Uj vizsgalat tipusa:\n [1] Covid-19 teszt\n [2] Tudogyulladas teszt\n [3] Kiutes\n [4] Influenza\n [5] Megfazas\n");
        int c = getchar();
        if(c == '1')
            strcpy(patient->testType, "Covid-19");
        else if(c == '2')
            strcpy(patient->testType, "Tudogyulladas");
        else if(c == '3')
            strcpy(patient->testType, "Kiutes");
        else if(c == '4')
            strcpy(patient->testType, "Influenza");
        else if(c == '5')
            strcpy(patient->testType, "Megfazas");
        else
            strcpy(patient->testType, "ERVENYTELEN");
        clearBuffer();
    }
    else if(c == '5')
    {
        printf("Igenyli-e az EXPRESS szolgaltatast?\n [1] IGEN\n [2] NEM\n");
        c = getchar();
        if(c == '1')
            strcpy(patient->express, "Igen");
        else if(c == '2')
            strcpy(patient->express, "Nem");
        else
            strcpy(patient->express, "?");
        clearBuffer();
    }      
    char *data[5] = {patient->name, patient->address, patient->TAJ, patient->testType, patient->express};
    int i;                     
    for(i=0; i<5; i++)
        cutNewLine(data[i]);

    fprintf(writeTo, "%s:%s:%s:%s:%s\n", patient->name, patient->address, patient->TAJ, patient->testType, patient->express);   //write the modificated data to the new file
    return 0;
}

void ambulanceSignalHandler(int signumber)
{
   printf("MUNKARA_FEL!\n"); 
}

void parentSignalHandler(int signumber)
{
    
}

void endSignalHandler(int signumber)
{
    printf("SIKERES_MINTAVETEL\n");
}