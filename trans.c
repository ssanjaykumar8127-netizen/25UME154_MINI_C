/****************************************************************
 * Project: Transaction Processing System
 * Course: 24UCS271 Lab Mini Project
 * Description: A system to manage banking records securely and efficiently.
 ****************************************************************/

// Bank-account program reads a random-access file sequentially,
// updates data already written to the file, creates new data to
// be placed in the file, and deletes data previously in the file.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// clientData structure definition
struct clientData
{
    unsigned int acctNum; // account number
    char lastName[15];    // account last name
    char firstName[10];   // account first name
    double balance;       // account balance
};                        // end structure clientData

// prototypes
unsigned int enterChoice(void);
void textFile(FILE *readPtr);
void updateRecord(FILE *fPtr);
void newRecord(FILE *fPtr);
void deleteRecord(FILE *fPtr);
void listRecords(FILE *fPtr); // prototype for listRecords
void transferFunds(FILE *fPtr); // prototype for transferFunds
void searchRecord(FILE *fPtr); // prototype for searchRecord
void viewRecord(FILE *fPtr); // prototype for efficient viewRecord
void bankStatistics(FILE *fPtr); // prototype for bank statistics

int main(int argc, char *argv[])
{
    FILE *cfPtr;         // credit.dat file pointer
    unsigned int choice; // user's choice

    // fopen opens the file; if it doesn't exist, create it
    if ((cfPtr = fopen("credit.dat", "rb+")) == NULL)
    {
        if ((cfPtr = fopen("credit.dat", "wb+")) == NULL) {
            printf("%s: File could not be opened.\n", argv[0]);
            exit(-1);
        } else {
            // initialize file with 100 empty records in a single write (faster)
            struct clientData blankClient = {0, "", "", 0.0};
            struct clientData blankRecords[100];
            for (int i = 0; i < 100; i++) blankRecords[i] = blankClient;
            fwrite(blankRecords, sizeof(struct clientData), 100, cfPtr);
            rewind(cfPtr);
        }
    }
    // use a larger I/O buffer for faster file operations
    setvbuf(cfPtr, NULL, _IOFBF, 65536);

    // enable user to specify action
    while ((choice = enterChoice()) != 10)
    {
        switch (choice)
        {
        // create text file from record file
        case 1:
            textFile(cfPtr);
            break;
        // update record
        case 2:
            updateRecord(cfPtr);
            break;
        // create record
        case 3:
            newRecord(cfPtr);
            break;
        // delete existing record
        case 4:
            deleteRecord(cfPtr);
            break;
        // list all accounts
        case 5:
            listRecords(cfPtr);
            break;
        // transfer funds
        case 6:
            transferFunds(cfPtr);
            break;
        // search record
        case 7:
            searchRecord(cfPtr);
            break;
        // view single record efficiently
        case 8:
            viewRecord(cfPtr);
            break;
        // calculate bank statistics
        case 9:
            bankStatistics(cfPtr);
            break;
        // display if user does not select valid choice
        default:
            puts("Incorrect choice");
            break;
        } // end switch
    }     // end while
    fclose(cfPtr); // fclose closes the file
} // end main

// create formatted text file for printing
void textFile(FILE *readPtr)
{
    FILE *writePtr; // accounts.txt file pointer
    int result;     // used to test whether fread read any bytes
    // create clientData with default information
    struct clientData client = {0, "", "", 0.0};

    // fopen opens the file; exits if file cannot be opened
    if ((writePtr = fopen("accounts.txt", "w")) == NULL)
    {
        puts("File could not be opened.");
    } // end if
    else
    {
        rewind(readPtr); // sets pointer to beginning of file
        fprintf(writePtr, "%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");

        // copy all records from random-access file into text file
        while (!feof(readPtr))
        {
            result = fread(&client, sizeof(struct clientData), 1, readPtr);

            // write single record to text file
            if (result != 0 && client.acctNum != 0)
            {
                fprintf(writePtr, "%-6d%-16s%-11s%10.2f\n", client.acctNum, client.lastName, client.firstName,
                        client.balance);
            } // end if
        }     // end while

        fclose(writePtr); // fclose closes the file
    }                     // end else
} // end function textFile

// update balance in record
void updateRecord(FILE *fPtr)
{
    unsigned int account; // account number
    double transaction;   // transaction amount
    // create clientData with no information
    struct clientData client = {0, "", "", 0.0};

    // obtain number of account to update
    printf("%s", "Enter account to update ( 1 - 100 ): ");
    scanf("%u", &account);

    if (account < 1 || account > 100) {
        puts("Invalid account number.");
        return;
    }

    // move file pointer to correct record in file
    fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
    // read record from file
    fread(&client, sizeof(struct clientData), 1, fPtr);
    // display error if account does not exist
    if (client.acctNum == 0)
    {
        printf("Account #%d has no information.\n", account);
    }
    else
    { // update record
        printf("%-6d%-16s%-11s%10.2f\n\n", client.acctNum, client.lastName, client.firstName, client.balance);

        // request transaction amount from user
        printf("%s", "Enter charge ( + ) or payment ( - ): ");
        scanf("%lf", &transaction);
        client.balance += transaction; // update record balance

        printf("%-6d%-16s%-11s%10.2f\n", client.acctNum, client.lastName, client.firstName, client.balance);

        // move file pointer to correct record in file
        // move back by 1 record length
        fseek(fPtr, -sizeof(struct clientData), SEEK_CUR);
        // write updated record over old record in file
        fwrite(&client, sizeof(struct clientData), 1, fPtr);
    } // end else
} // end function updateRecord

// delete an existing record
void deleteRecord(FILE *fPtr)
{
    struct clientData client;                       // stores record read from file
    struct clientData blankClient = {0, "", "", 0}; // blank client
    unsigned int accountNum;                        // account number

    // obtain number of account to delete
    printf("%s", "Enter account number to delete ( 1 - 100 ): ");
    scanf("%u", &accountNum);

    if (accountNum < 1 || accountNum > 100) {
        puts("Invalid account number.");
        return;
    }

    // move file pointer to correct record in file
    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    // read record from file
    fread(&client, sizeof(struct clientData), 1, fPtr);
    // display error if record does not exist
    if (client.acctNum == 0)
    {
        printf("Account %d does not exist.\n", accountNum);
    } // end if
    else
    { // delete record
        // move file pointer to correct record in file
        fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
        // replace existing record with blank record
        fwrite(&blankClient, sizeof(struct clientData), 1, fPtr);
    } // end else
} // end function deleteRecord

// create and insert record
void newRecord(FILE *fPtr)
{
    // create clientData with default information
    struct clientData client = {0, "", "", 0.0};
    unsigned int accountNum; // account number

    // obtain number of account to create
    printf("%s", "Enter new account number ( 1 - 100 ): ");
    scanf("%u", &accountNum);

    if (accountNum < 1 || accountNum > 100) {
        puts("Invalid account number.");
        return;
    }

    // move file pointer to correct record in file
    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    // read record from file
    fread(&client, sizeof(struct clientData), 1, fPtr);
    // display error if account already exists
    if (client.acctNum != 0)
    {
        printf("Account #%d already contains information.\n", client.acctNum);
    } // end if
    else
    { // create record
        // user enters last name, first name and balance
        printf("%s", "Enter lastname, firstname, balance\n? ");
        scanf("%14s%9s%lf", client.lastName, client.firstName, &client.balance);

        client.acctNum = accountNum;
        // move file pointer to correct record in file
        fseek(fPtr, (client.acctNum - 1) * sizeof(struct clientData), SEEK_SET);
        // insert record in file
        fwrite(&client, sizeof(struct clientData), 1, fPtr);
    } // end else
} // end function newRecord

// enable user to input menu choice
unsigned int enterChoice(void)
{
    unsigned int menuChoice; // variable to store user's choice
    // display available options
    printf("%s", "\nEnter your choice\n"
                 "1 - store a formatted text file of accounts called\n"
                 "    \"accounts.txt\" for printing\n"
                 "2 - update an account\n"
                 "3 - add a new account\n"
                 "4 - delete an account\n"
                 "5 - list all accounts\n"
                 "6 - transfer funds\n"
                 "7 - search account by last name\n"
                 "8 - view account by ID (Efficient O(1) lookup)\n"
                 "9 - bank statistics\n"
                 "10 - end program\n? ");

    if (scanf("%u", &menuChoice) != 1) {
        // clear input buffer to prevent infinite loops if char is entered
        while (getchar() != '\n');
        menuChoice = 0; // return invalid choice
    }
    return menuChoice;
} // end function enterChoice

// list all account information
void listRecords(FILE *fPtr)
{
    struct clientData client = {0, "", "", 0.0};
    int result;

    rewind(fPtr); // sets pointer to beginning of file
    printf("\n%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");
    printf("---------------------------------------------\n");

    while (!feof(fPtr))
    {
        result = fread(&client, sizeof(struct clientData), 1, fPtr);

        // write single record to console
        if (result != 0 && client.acctNum != 0)
        {
            printf("%-6d%-16s%-11s%10.2f\n", client.acctNum, client.lastName, client.firstName, client.balance);
        }
    }
} // end function listRecords

// transfer funds between two accounts
void transferFunds(FILE *fPtr)
{
    unsigned int srcAccount, destAccount;
    double amount;
    struct clientData srcClient = {0, "", "", 0.0};
    struct clientData destClient = {0, "", "", 0.0};

    printf("%s", "Enter source account ( 1 - 100 ): ");
    scanf("%u", &srcAccount);
    if (srcAccount < 1 || srcAccount > 100) {
        puts("Invalid source account number.");
        return;
    }

    printf("%s", "Enter destination account ( 1 - 100 ): ");
    scanf("%u", &destAccount);
    if (destAccount < 1 || destAccount > 100) {
        puts("Invalid destination account number.");
        return;
    }

    // Read source account
    fseek(fPtr, (srcAccount - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&srcClient, sizeof(struct clientData), 1, fPtr);
    if (srcClient.acctNum == 0) {
        printf("Source account #%u has no information.\n", srcAccount);
        return;
    }

    // Read destination account
    fseek(fPtr, (destAccount - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&destClient, sizeof(struct clientData), 1, fPtr);
    if (destClient.acctNum == 0) {
        printf("Destination account #%u has no information.\n", destAccount);
        return;
    }

    printf("Enter transfer amount: ");
    scanf("%lf", &amount);
    
    if (amount <= 0) {
        puts("Transfer amount must be positive.");
        return;
    }
    
    if (srcClient.balance < amount) {
        puts("Insufficient funds in source account.");
        return;
    }

    // Perform transfer
    srcClient.balance -= amount;
    destClient.balance += amount;

    // Write source account back
    fseek(fPtr, (srcAccount - 1) * sizeof(struct clientData), SEEK_SET);
    fwrite(&srcClient, sizeof(struct clientData), 1, fPtr);

    // Write destination account back
    fseek(fPtr, (destAccount - 1) * sizeof(struct clientData), SEEK_SET);
    fwrite(&destClient, sizeof(struct clientData), 1, fPtr);
    
    printf("Successfully transferred %.2f from account %u to account %u.\n", amount, srcAccount, destAccount);
} // end function transferFunds

// search for an account by last name
void searchRecord(FILE *fPtr)
{
    char searchName[15];
    struct clientData client = {0, "", "", 0.0};
    int found = 0;

    printf("%s", "Enter last name to search for: ");
    scanf("%14s", searchName);

    rewind(fPtr); // sets pointer to beginning of file
    printf("\n%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");
    printf("---------------------------------------------\n");

    while (!feof(fPtr))
    {
        int result = fread(&client, sizeof(struct clientData), 1, fPtr);

        // write single record to console if name matches
        if (result != 0 && client.acctNum != 0 && strcmp(client.lastName, searchName) == 0)
        {
            printf("%-6d%-16s%-11s%10.2f\n", client.acctNum, client.lastName, client.firstName, client.balance);
            found = 1;
        }
    }

    if (!found) {
        printf("No accounts found with last name '%s'.\n", searchName);
    }
} // end function searchRecord

// view a single record using efficient O(1) lookup
void viewRecord(FILE *fPtr)
{
    unsigned int accountNum;
    struct clientData client = {0, "", "", 0.0};

    // obtain number of account to view
    printf("%s", "Enter account number to view ( 1 - 100 ): ");
    scanf("%u", &accountNum);

    if (accountNum < 1 || accountNum > 100) {
        puts("Invalid account number.");
        return;
    }

    // move file pointer to correct record in file - O(1) efficiency
    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    
    // read record from file
    fread(&client, sizeof(struct clientData), 1, fPtr);
    
    // display error if record does not exist
    if (client.acctNum == 0)
    {
        printf("Account %u does not exist.\n", accountNum);
    }
    else
    {
        printf("\n%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");
        printf("---------------------------------------------\n");
        printf("%-6d%-16s%-11s%10.2f\n", client.acctNum, client.lastName, client.firstName, client.balance);
    }
} // end function viewRecord

// calculate and display bank statistics
void bankStatistics(FILE *fPtr)
{
    struct clientData client = {0, "", "", 0.0};
    int activeAccountsCount = 0;
    double totalDeposits = 0.0;

    rewind(fPtr); // sets pointer to beginning of file

    while (!feof(fPtr))
    {
        int result = fread(&client, sizeof(struct clientData), 1, fPtr);

        if (result != 0 && client.acctNum != 0)
        {
            activeAccountsCount++;
            totalDeposits += client.balance;
        }
    }

    printf("\n==== BANK STATISTICS ====\n");
    printf("Total Active Accounts: %d\n", activeAccountsCount);
    printf("Total Bank Deposits:   %.2f\n", totalDeposits);
    printf("=========================\n");
} // end function bankStatistics