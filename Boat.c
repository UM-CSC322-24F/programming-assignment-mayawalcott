#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_BOATS 120
#define MAX_BOAT_NAME_LEN 127
#define MAX_LINE_LENGTH 512


// which type of place (slip/land/trailer/storage)
typedef enum {
    slip,
    land,
    trailer,
    storage,
} PlaceType;

// union to hold extra relevant information
typedef union {
    int slipNumber;
    char bayLetter;
    char trailerTag[MAX_BOAT_NAME_LEN + 1];
    int storageNumber;
} BoatData;

// contains boat information
typedef struct {
    char boatName[MAX_BOAT_NAME_LEN + 1];
    int lengthInFeet;
    PlaceType type;
    BoatData extraData;
    float moneyOwed;
} Boat;

// these are all of my function prototypes
void boatDataLoader(const char* filename, Boat** boats, int* counter);
int compareBoats(const void *pointer1, const void *pointer2);
void printInventory(Boat **boats, int counter);
void addBoat(int* counter, Boat **boats);
void removeBoat(int *counter, Boat **boats);
void acceptAPayment(int *counter, Boat **boats);
void updateOwedMonthly(int counter, Boat **boats);
void saveBoatData(const char* filename, Boat** boats, int counter);

//parameters allow for us to account for some command line arguments
int main(int argc, char *argv[]) {
    int counter = 0;

    //checking if the underlying argument count is 2
    if (argc != 2) {
        perror("File name not given!\n");
        return(EXIT_FAILURE);
    }

    Boat *boats[MAX_BOATS] = {NULL};

    //load boatData from CSV
    boatDataLoader(argv[1], boats, &counter);

    char choice;
    printf("Welcome to the Boat Management System\n");
    printf("-------------------------------------\n");

    do {
        printf("(I)nventory, (A)dd, (R)emove, (P)ayment, (M)onth, e(X)it : ");
        scanf(" %c", &choice);
	//case insensitivity
	choice = toupper(choice);

	while (getchar() != '\n');

        switch(choice) {
            case 'I':
                printInventory(boats, counter);
                break;
            case 'A':
                addBoat(&counter, boats);
                break;
            case 'R':
                removeBoat(&counter, boats);
                break;
            case 'P':
                acceptAPayment(&counter, boats);
                break;
            case 'M':
                updateOwedMonthly(counter, boats);
                break;
            case 'X':
                saveBoatData(argv[1], boats, counter);
                break;
            default:
                break;
        }

    } while (choice != 'X');

    printf("Exiting the Boat Management System\n");

    exit(EXIT_SUCCESS);
}

///*the purpose of this it to load the data from the file once the program starts
//whose name is provided as the command line argument, and when the program exits it must store the data back to that file*/
void boatDataLoader(const char* filename, Boat** boats, int* counter) {
    char line[MAX_LINE_LENGTH];
    FILE *file = fopen(filename, "r");

    if (!file) {
        printf("Error opening file!\n");
        exit(EXIT_FAILURE);
    }

    while (fgets(line, sizeof(line), file) && *counter < MAX_BOATS) {
        Boat *boat = malloc(sizeof(Boat));
        if (!boat) {
            perror("Memory allocation failed");
            fclose(file);
            exit(EXIT_FAILURE);
        }

        char type[15];
	
        sscanf(line, "%127[^,],%d,%[^,],%[^,],%f",
               boat->boatName,
               &boat->lengthInFeet,
               type,
               (char*)&boat->extraData,
               &boat->moneyOwed);

	boat->boatName[strcspn(boat->boatName, "\n")] = 0;

        if (strcmp(type, "slip") == 0) {
            boat->type = slip;
            sscanf(line, "%127[^,],%d,slip,%d,%f", boat->boatName, &boat->lengthInFeet, &boat->extraData.slipNumber, &boat->moneyOwed);
        } else if (strcmp(type, "land") == 0) {
            boat->type = land;
            sscanf(line, "%127[^,],%d,land,%c,%f", boat->boatName, &boat->lengthInFeet, &boat->extraData.bayLetter, &boat->moneyOwed);
        } else if (strcmp(type, "trailer") == 0) {
            boat->type = trailer;
            sscanf(line, "%127[^,],%d,trailer,%127s,%f", boat->boatName, &boat->lengthInFeet, boat->extraData.trailerTag, &boat->moneyOwed);
        } else if (strcmp(type, "storage") == 0) {
            boat->type = storage;
            sscanf(line, "%127[^,],%d,storage,%d,%f", boat->boatName, &boat->lengthInFeet, &boat->extraData.storageNumber, &boat->moneyOwed);
        } else {
            free(boat);
            continue; // Skip invalid entries
        }

        boats[(*counter)++] = boat;
    }

    fclose(file);
}

//defining the compare functions so we can use it in qsort
int compareBoats(const void *pointer1, const void *pointer2) {
    return strcmp((*(Boat **)pointer1)->boatName, (*(Boat **)pointer2)->boatName);
}

//purpose is to print boat inventory
void printInventory(Boat **boats, int counter) {
    qsort(boats, counter, sizeof(Boat *), compareBoats);

    for (int i = 0; i < counter; i++) {
        Boat *boat = boats[i];
        printf("%-20s %3d' %10s ", boat->boatName, boat->lengthInFeet,
               boat->type == slip ? "slip" :
               boat->type == land ? "land" :
               boat->type == trailer ? "trailer" : "storage");

        switch (boat->type) {
            case slip: printf("# %d", boat->extraData.slipNumber); break;
            case land: printf("%c", boat->extraData.bayLetter); break;
            case trailer: printf("%s", boat->extraData.trailerTag); break;
            case storage: printf("# %d", boat->extraData.storageNumber); break;
        } //end of switch

        printf(" Owes $%.2f\n", boat->moneyOwed);
    } //end of function
}

//the purpose of this function is to be able to add a boat
void addBoat(int* counter, Boat **boats) {
    if (*counter >= MAX_BOATS) {
        perror("Cannot add any more boats. Program only allows for up to 120 boats.\n");
        exit(EXIT_FAILURE);
    }

    Boat *boat = malloc(sizeof(Boat));
    if (!boat) {
        perror("Memory Allocation has failed\n");
        exit(EXIT_FAILURE);
    }

    char type[15];
    printf("Please enter the boat data in CSV format: ");
    scanf(" %127[^,],%d,%[^,],%[^,],%f", boat->boatName, &boat->lengthInFeet, type,
          (char*)&boat->extraData, &boat->moneyOwed);

    if (strcmp(type, "slip") == 0) {
        boat->type = slip;
        sscanf(type, "%127[^,],%d,slip,%d,%f", boat->boatName, &boat->lengthInFeet, &boat->extraData.slipNumber, &boat->moneyOwed);
    } else if (strcmp(type, "land") == 0) {
        boat->type = land;
    } else if (strcmp(type, "trailer") == 0) {
        boat->type = trailer;
    } else if (strcmp(type, "storage") == 0) {
        boat->type = storage;
    } else {
        printf("Error. Boat has not been added\n");
        free(boat);
        return;
    }

    boats[(*counter)++] = boat;
    printf("Boat has been added\n");
}

//the purpose is to remove a boat by name
void removeBoat(int *counter, Boat **boats) {
    char boatName[MAX_BOAT_NAME_LEN + 1];
    printf("Please enter the boat name: ");
    scanf(" %127s", boatName);

    int found = 0;

    for (int i = 0; i < *counter; i++) {
        if (strcasecmp(boats[i]->boatName, boatName) == 0) {
            free(boats[i]);
            boats[i] = boats[--(*counter)];
            found = 1;
            printf("Success! Boat has been removed.\n");
            break;
        }
    }

    if (!found) {
        printf("No boat with that name\n");
	return;
    }
}

void acceptAPayment(int *counter, Boat **boats) {
    char boatName[MAX_BOAT_NAME_LEN + 1];
    float payment;

    printf("Please enter the boat name: ");
    scanf(" %127s", boatName);
    printf("Please enter the amount to be paid: ");
    scanf("%f", &payment);

    for (int i = 0; i < *counter; i++) {
        if (strcasecmp(boats[i]->boatName, boatName) == 0) {
            if (payment > boats[i]->moneyOwed) {
                printf("That is more than the amount owed, $%.2f\n", boats[i]->moneyOwed);
                return;
            }
            boats[i]->moneyOwed -= payment;
            printf("Payment accepted. New amount owed: $%.2f\n", boats[i]->moneyOwed);
            return;
        }
    }
    printf("No boat with that name\n");
}

void updateOwedMonthly(int counter, Boat **boats) {
    for (int i = 0; i < counter; i++) {
        boats[i]->moneyOwed += 10.0; // Increase by $10.00 monthly
    }
    printf("Monthly fees updated.\n");
}

//this is important in order to be able to save the Boat Data
void saveBoatData(const char* filename, Boat** boats, int counter) {
    FILE *file = fopen(filename, "w");

    if (!file) {
        printf("Error opening file for writing!\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < counter; i++) {
        Boat *boat = boats[i];
        fprintf(file, "%s,%d,", boat->boatName, boat->lengthInFeet);
        switch (boat->type) {
            case slip:
                fprintf(file, "slip,%d,%.2f\n", boat->extraData.slipNumber, boat->moneyOwed);
                break;
            case land:
                fprintf(file, "land,%c,%.2f\n", boat->extraData.bayLetter, boat->moneyOwed);
                break;
            case trailer:
                fprintf(file, "trailer,%s,%.2f\n", boat->extraData.trailerTag, boat->moneyOwed);
                break;
            case storage:
                fprintf(file, "storage,%d,%.2f\n", boat->extraData.storageNumber, boat->moneyOwed);
                break;
        }
    }

    fclose(file);
    printf("Data saved successfully.\n");
}

