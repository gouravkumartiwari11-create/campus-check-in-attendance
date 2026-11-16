#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_STUDENTS 100
#define NAME_LENGTH 50
#define LINE_LENGTH 256
#define STUDENT_FILE "students.txt"
#define ATTENDANCE_FILE "attendance.txt"

struct Student {
    int roll;
    char name[NAME_LENGTH];
    char pin[5];
};

struct Attendance {
    char date[11];
    int roll;
    char time[9];
};

static struct Student students[MAX_STUDENTS];
static int student_count = 0;

static int read_line(const char *prompt, char *text, size_t size)
{
    size_t length;

    for (;;) {
        printf("%s", prompt);
        if (fgets(text, (int)size, stdin) == NULL) {
            return 0;
        }

        length = strcspn(text, "\n");
        if (text[length] != '\n' && !feof(stdin)) {
            int character;
            while ((character = getchar()) != '\n' && character != EOF) {
                /* Discard the rest of an overlong input line. */
            }
            puts("That input is too long. Please try again.");
            continue;
        }

        text[length] = '\0';
        if (length > 0 && text[length - 1] == '\r') {
            text[length - 1] = '\0';
        }
        return 1;
    }
}

static int read_integer(const char *prompt, int minimum, int maximum, int *value)
{
    char text[64];
    char *start;
    char *end;
    long parsed;

    for (;;) {
        if (!read_line(prompt, text, sizeof(text))) {
            return 0;
        }

        errno = 0;
        start = text;
        while (isspace((unsigned char)*start)) {
            start++;
        }
        parsed = strtol(start, &end, 10);
        while (isspace((unsigned char)*end)) {
            end++;
        }
        if (start != end && *end == '\0' && errno != ERANGE &&
            parsed >= minimum && parsed <= maximum) {
            *value = (int)parsed;
            return 1;
        }
        printf("Enter a whole number from %d to %d.\n", minimum, maximum);
    }
}

static int read_pin(const char *prompt, char pin[5])
{
    char text[64];

    for (;;) {
        if (!read_line(prompt, text, sizeof(text))) {
            return 0;
        }
        if (strlen(text) == 4 &&
            isdigit((unsigned char)text[0]) &&
            isdigit((unsigned char)text[1]) &&
            isdigit((unsigned char)text[2]) &&
            isdigit((unsigned char)text[3])) {
            memcpy(pin, text, 5);
            return 1;
        }
        puts("Use exactly four digits for the PIN.");
    }
}

static int read_stored_line(FILE *file, char *line, size_t size)
{
    size_t length;

    if (fgets(line, (int)size, file) == NULL) {
        return feof(file) ? 0 : -1;
    }

    length = strcspn(line, "\n");
    if (line[length] != '\n' && !feof(file)) {
        int character;
        while ((character = fgetc(file)) != '\n' && character != EOF) {
            /* Discard the rest of a damaged record. */
        }
        return -1;
    }

    line[length] = '\0';
    if (length > 0 && line[length - 1] == '\r') {
        line[length - 1] = '\0';
    }
    return 1;
}

static int valid_date(const char *date)
{
    int i;

    if (strlen(date) != 10 || date[4] != '-' || date[7] != '-') {
        return 0;
    }
    for (i = 0; i < 10; i++) {
        if (i != 4 && i != 7 && !isdigit((unsigned char)date[i])) {
            return 0;
        }
    }
    return 1;
}

static int valid_time(const char *time_text)
{
    int i;

    if (strlen(time_text) != 8 || time_text[2] != ':' || time_text[5] != ':') {
        return 0;
    }
    for (i = 0; i < 8; i++) {
        if (i != 2 && i != 5 && !isdigit((unsigned char)time_text[i])) {
            return 0;
        }
    }
    return 1;
}

static int parse_student(const char *line, struct Student *student)
{
    char extra;
    int fields;

    fields = sscanf(line, "%d|%4[0-9]|%49[^|\r\n]%c",
                    &student->roll, student->pin, student->name, &extra);
    return fields == 3 && student->roll > 0 &&
           strlen(student->pin) == 4 && student->name[0] != '\0';
}

static int parse_attendance(const char *line, struct Attendance *record)
{
    char extra;
    int fields;

    fields = sscanf(line, "%10[^|]|%d|%8[^|\r\n]%c",
                    record->date, &record->roll, record->time, &extra);
    return fields == 3 && valid_date(record->date) &&
           record->roll > 0 && valid_time(record->time);
}

static int find_student(int roll)
{
    int i;

    for (i = 0; i < student_count; i++) {
        if (students[i].roll == roll) {
            return i;
        }
    }
    return -1;
}

static int ensure_data_file(const char *filename)
{
    FILE *file = fopen(filename, "a");

    if (file == NULL) {
        fprintf(stderr, "Cannot open %s. Check the folder permissions.\n", filename);
        return 0;
    }
    if (fclose(file) != 0) {
        fprintf(stderr, "Could not close %s safely.\n", filename);
        return 0;
    }
    return 1;
}

static int load_students(void)
{
    FILE *file = fopen(STUDENT_FILE, "r");
    char line[LINE_LENGTH];
    int line_number = 0;
    int status;

    if (file == NULL) {
        fprintf(stderr, "Cannot read %s.\n", STUDENT_FILE);
        return 0;
    }

    while ((status = read_stored_line(file, line, sizeof(line))) != 0) {
        struct Student student;
        int i;

        line_number++;
        if (status < 0) {
            fprintf(stderr, "A record in %s is too long or could not be read.\n",
                    STUDENT_FILE);
            fclose(file);
            return 0;
        }
        if (!parse_student(line, &student)) {
            fprintf(stderr, "Skipping invalid student record on line %d.\n", line_number);
            continue;
        }

        for (i = 0; i < student_count; i++) {
            if (students[i].roll == student.roll) {
                break;
            }
        }
        if (i != student_count) {
            fprintf(stderr, "Skipping duplicate roll number on line %d.\n", line_number);
            continue;
        }
        if (student_count == MAX_STUDENTS) {
            fprintf(stderr, "Student limit reached; remaining records were not loaded.\n");
            break;
        }
        students[student_count++] = student;
    }

    if (ferror(file)) {
        fprintf(stderr, "An error occurred while reading %s.\n", STUDENT_FILE);
        fclose(file);
        return 0;
    }
    if (fclose(file) != 0) {
        fprintf(stderr, "Could not close %s safely.\n", STUDENT_FILE);
        return 0;
    }
    return 1;
}

static int get_today(char date[11], char time_text[9])
{
    time_t now = time(NULL);
    struct tm *local;

    if (now == (time_t)-1) {
        fprintf(stderr, "The current date and time could not be read.\n");
        return 0;
    }
    local = localtime(&now);
    if (local == NULL ||
        strftime(date, 11, "%Y-%m-%d", local) == 0 ||
        strftime(time_text, 9, "%H:%M:%S", local) == 0) {
        fprintf(stderr, "The current date and time could not be read.\n");
        return 0;
    }
    return 1;
}

static int check_in_exists(const char *date, int roll, int *exists)
{
    FILE *file = fopen(ATTENDANCE_FILE, "r");
    char line[LINE_LENGTH];
    int status;

    *exists = 0;
    if (file == NULL) {
        fprintf(stderr, "Cannot read %s.\n", ATTENDANCE_FILE);
        return 0;
    }

    while ((status = read_stored_line(file, line, sizeof(line))) != 0) {
        struct Attendance record;

        if (status < 0 || !parse_attendance(line, &record)) {
            fprintf(stderr, "Invalid attendance record found; check-in stopped.\n");
            fclose(file);
            return 0;
        }
        if (record.roll == roll && strcmp(record.date, date) == 0) {
            *exists = 1;
        }
    }

    if (ferror(file)) {
        fprintf(stderr, "An error occurred while reading %s.\n", ATTENDANCE_FILE);
        fclose(file);
        return 0;
    }
    if (fclose(file) != 0) {
        fprintf(stderr, "Could not close %s safely.\n", ATTENDANCE_FILE);
        return 0;
    }
    return 1;
}

static void show_banner(void)
{
    puts("\n+--------------------------------------------------+");
    puts("|          CAMPUS CHECK-IN - ATTENDANCE           |");
    puts("|        A small C programming class project       |");
    puts("+--------------------------------------------------+");
}

static void enroll_student(void)
{
    struct Student student;
    char confirm_pin[5];
    char name_input[64];
    int roll;
    int i;
    FILE *file;

    if (student_count == MAX_STUDENTS) {
        puts("The class list is full (maximum 100 students).");
        return;
    }

    if (!read_integer("New student's roll number: ", 1, INT_MAX, &roll)) {
        return;
    }
    if (find_student(roll) != -1) {
        puts("That roll number is already registered.");
        return;
    }

    if (!read_line("Student name: ", name_input, sizeof(name_input))) {
        return;
    }
    if (strlen(name_input) >= sizeof(student.name)) {
        puts("Name is too long (maximum 49 characters).");
        return;
    }
    if (name_input[0] == '\0' ||
        strspn(name_input, " \t") == strlen(name_input) ||
        strchr(name_input, '|') != NULL) {
        puts("Enter a name and do not use the | character.");
        return;
    }
    strcpy(student.name, name_input);
    student.roll = roll;

    for (i = 0; i < 3; i++) {
        if (!read_pin("Create a private four-digit student PIN: ", student.pin)) {
            return;
        }
        if (!read_pin("Enter the PIN again: ", confirm_pin)) {
            return;
        }
        if (strcmp(student.pin, confirm_pin) == 0) {
            break;
        }
        puts("The PINs did not match. Try again.");
    }
    if (i == 3) {
        puts("Student was not enrolled. Please start again.");
        return;
    }

    file = fopen(STUDENT_FILE, "a");
    if (file == NULL) {
        fprintf(stderr, "Could not save the student to %s.\n", STUDENT_FILE);
        return;
    }
    if (fprintf(file, "%d|%s|%s\n", student.roll, student.pin, student.name) < 0) {
        fprintf(stderr, "Could not write the student record.\n");
        fclose(file);
        return;
    }
    if (fclose(file) != 0) {
        fprintf(stderr, "Could not finish saving the student record.\n");
        return;
    }

    students[student_count++] = student;
    puts("Student enrolled. Keep the PIN private.");
}

static void student_check_in(void)
{
    int roll;
    int index;
    int exists;
    int attempt;
    char pin[5];
    char date[11];
    char time_text[9];
    FILE *file;

    if (student_count == 0) {
        puts("No students are enrolled yet. Ask the instructor to add the class list.");
        return;
    }
    if (!read_integer("Your roll number: ", 1, INT_MAX, &roll)) {
        return;
    }
    index = find_student(roll);
    if (index == -1) {
        puts("That roll number is not on the class list.");
        return;
    }

    for (attempt = 0; attempt < 3; attempt++) {
        if (!read_pin("Your private PIN: ", pin)) {
            return;
        }
        if (strcmp(pin, students[index].pin) == 0) {
            break;
        }
        puts("Incorrect PIN.");
    }
    if (attempt == 3) {
        puts("Check-in stopped after three incorrect PINs.");
        return;
    }

    if (!get_today(date, time_text) ||
        !check_in_exists(date, roll, &exists)) {
        return;
    }
    if (exists) {
        printf("%s is already marked present for today.\n", students[index].name);
        return;
    }

    file = fopen(ATTENDANCE_FILE, "a");
    if (file == NULL) {
        fprintf(stderr, "Could not save attendance to %s.\n", ATTENDANCE_FILE);
        return;
    }
    if (fprintf(file, "%s|%d|%s\n", date, roll, time_text) < 0) {
        fprintf(stderr, "Could not write the attendance record.\n");
        fclose(file);
        return;
    }
    if (fclose(file) != 0) {
        fprintf(stderr, "Could not finish saving the attendance record.\n");
        return;
    }

    printf("Check-in saved for %s at %s. Have a good class!\n",
           students[index].name, time_text);
}

static void show_daily_report(void)
{
    int present[MAX_STUDENTS] = {0};
    int present_count = 0;
    int i;
    char date[11];
    char time_text[9];
    char line[LINE_LENGTH];
    int status;
    FILE *file;

    if (student_count == 0) {
        puts("No students are enrolled yet.");
        return;
    }
    if (!get_today(date, time_text)) {
        return;
    }

    file = fopen(ATTENDANCE_FILE, "r");
    if (file == NULL) {
        fprintf(stderr, "Cannot read %s.\n", ATTENDANCE_FILE);
        return;
    }
    while ((status = read_stored_line(file, line, sizeof(line))) != 0) {
        struct Attendance record;
        int index;

        if (status < 0 || !parse_attendance(line, &record)) {
            fprintf(stderr, "Invalid attendance record found; report stopped.\n");
            fclose(file);
            return;
        }
        if (strcmp(record.date, date) == 0) {
            index = find_student(record.roll);
            if (index != -1 && !present[index]) {
                present[index] = 1;
                present_count++;
            }
        }
    }
    if (ferror(file)) {
        fprintf(stderr, "An error occurred while reading %s.\n", ATTENDANCE_FILE);
        fclose(file);
        return;
    }
    if (fclose(file) != 0) {
        fprintf(stderr, "Could not close %s safely.\n", ATTENDANCE_FILE);
        return;
    }

    printf("\nDAILY ATTENDANCE - %s\n", date);
    puts("--------------------------------------------------");
    printf("%-8s %-30s %s\n", "ROLL", "STUDENT", "STATUS");
    puts("--------------------------------------------------");
    for (i = 0; i < student_count; i++) {
        printf("%-8d %-30s %s\n", students[i].roll, students[i].name,
               present[i] ? "PRESENT" : "ABSENT");
    }
    puts("--------------------------------------------------");
    printf("Present: %d / %d    Absent: %d\n",
           present_count, student_count, student_count - present_count);
}

static void show_student_list(void)
{
    int i;

    if (student_count == 0) {
        puts("No students are enrolled yet.");
        return;
    }
    puts("\nCLASS LIST");
    puts("------------------------------------------");
    printf("%-8s %s\n", "ROLL", "STUDENT");
    puts("------------------------------------------");
    for (i = 0; i < student_count; i++) {
        printf("%-8d %s\n", students[i].roll, students[i].name);
    }
}

int main(void)
{
    int choice;

    if (!ensure_data_file(STUDENT_FILE) ||
        !ensure_data_file(ATTENDANCE_FILE) ||
        !load_students()) {
        return 1;
    }

    show_banner();
    for (;;) {
        puts("\n1. Student check-in");
        puts("2. Enroll a student (instructor)");
        puts("3. View today's attendance");
        puts("4. View class list");
        puts("0. Exit");
        if (!read_integer("Choose an option: ", 0, 4, &choice)) {
            puts("\nGoodbye!");
            break;
        }

        switch (choice) {
        case 1:
            student_check_in();
            break;
        case 2:
            enroll_student();
            break;
        case 3:
            show_daily_report();
            break;
        case 4:
            show_student_list();
            break;
        case 0:
            puts("Goodbye! See you in class.");
            return 0;
        }
    }
    return 0;
}
