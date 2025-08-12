#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#include "common.h"
#include "parse.h"

int delete_employee(struct dbheader_t *dbhdr, struct employee_t *employees, char *deletestring){
	int employee_index = search_employee(dbhdr, employees, deletestring);
	if(employee_index == STATUS_ERROR){
		printf("Cannot delete an employee that does not exist\n");
		return employee_index;
	}

	printf("Deleting employee: %s\n", employees[employee_index].name);

    for(int i = employee_index; i < dbhdr->count - 1; i++){
        employees[i] = employees[i + 1];
    }
    
    dbhdr->count--;
    dbhdr->filesize = sizeof(struct dbheader_t) + (dbhdr->count * sizeof(struct employee_t));
    
    printf("Employee deleted successfully\n");
    return STATUS_SUCCESS;

}

int search_employee(struct dbheader_t *dbhdr, struct employee_t *employees, char *searchstring){
	int i = 0;
	for(; i < dbhdr->count; i++){
		if(strcasecmp(employees[i].name, searchstring) == 0){
			printf("Employee %d Found\n", i+1);
			printf("\tName: %s\n", employees[i].name);
			printf("\tAddress: %s\n", employees[i].address);
			printf("\tHours: %d\n", employees[i].hours);
			return i;
		}
	}

	printf("No Employee with that name found\n");
	return STATUS_ERROR;
}

void list_employees(struct dbheader_t *dbhdr, struct employee_t *employees){
	int i=0;
	for(; i < dbhdr->count; i++){
		printf("Employee %d\n", i+1);
		printf("\tName %s\n", employees[i].name);
		printf("\tAddress %s\n", employees[i].address);
		printf("\thours %d\n", employees[i].hours);
	}
}


int add_employee(struct dbheader_t *dbhdr, struct employee_t *employees, char *addstring) {
	char *name = strtok(addstring, ",");

	char *addr = strtok(NULL, ",");

	char *hours = strtok(NULL, ",");

	printf("%s %s %s\n", name, addr, hours);

	strncpy(employees[dbhdr->count-1].name, name, sizeof(employees[dbhdr->count-1].name));

	strncpy(employees[dbhdr->count-1].address, addr, sizeof(employees[dbhdr->count-1].address));

	employees[dbhdr->count-1].hours = atoi(hours);

	return STATUS_SUCCESS;
}

int read_employees(int fd, struct dbheader_t *dbhdr, struct employee_t **employeesOut){
	if (fd <0){
		printf("Got a bad FD from the user\n");
		return STATUS_ERROR;
	}

	int count = dbhdr->count;

	struct employee_t *employees = calloc(count, sizeof(struct employee_t));
	if(employees == NULL){
		printf("Malloc failed\n");
		return STATUS_ERROR;
	}

	read(fd, employees, count*sizeof(struct employee_t));

	int i = 0;
	for(; i < count; i++) {
		employees[i].hours = ntohl(employees[i].hours);
	}

	*employeesOut = employees;
	return STATUS_SUCCESS;
}

void output_file(int fd, struct dbheader_t *dbhdr, struct employee_t *employees) {
	if (fd <0){
		printf("Got a bad FD from the user\n");
		return;
	}

	int realcount = dbhdr->count;
	int new_filesize = sizeof(struct dbheader_t) + (sizeof(struct employee_t) * realcount);

    struct dbheader_t header_copy = *dbhdr;
    header_copy.magic = htonl(header_copy.magic);
    header_copy.filesize = htonl(new_filesize);
    header_copy.count = htons(header_copy.count);
    header_copy.version = htons(header_copy.version);

    lseek(fd, 0, SEEK_SET);
    write(fd, &header_copy, sizeof(struct dbheader_t));

	int i = 0;
	for(; i < realcount; i++) {
		employees[i].hours = htonl(employees[i].hours);
		write(fd, &employees[i], sizeof(struct employee_t));
	}

	ftruncate(fd, new_filesize);

	return;
}

int validate_db_header(int fd, struct dbheader_t ** headerOut) {
	if (fd <0){
		printf("Got a bad FD from the user\n");
		return STATUS_ERROR;
	}

	struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
	if (header == NULL){
		printf("Malloc failed to create db header\n");
		return STATUS_ERROR;
	};

	if (read(fd, header, sizeof(struct dbheader_t)) != sizeof(struct dbheader_t)) {
		perror("read");
		free(header);
		return STATUS_ERROR;
	}

	header->version = ntohs(header->version);
	header->count = ntohs(header->count);
	header->magic = ntohl(header->magic);
	header->filesize = ntohl(header->filesize);


	if(header->magic != HEADER_MAGIC){
		printf("Improper header magic\n");
		free(header);
		return -1;
	}

	if(header->version != 1){
		printf("Improper header version\n");
		free(header);
		return -1;
	}

	struct stat dbstat = {0};
	fstat(fd, &dbstat);
	if(header->filesize != dbstat.st_size) {
		printf("Corrupted db\n");
		free(header);
		return -1;
	}

	*headerOut = header;
}

int create_db_header(int fd, struct dbheader_t ** headerOut){
	struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
	if (header == NULL){
		printf("Malloc failed to create db header\n");
		return STATUS_ERROR;
	};

	header->version = 0x1;
	header->count = 0;
	header->magic = HEADER_MAGIC;
	header->filesize = sizeof(struct dbheader_t);

	*headerOut = header;

	return STATUS_SUCCESS;
}