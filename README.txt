CAMPUS CHECK-IN - STUDENT ATTENDANCE MANAGEMENT
================================================

This is a small C console project for the Programming for Problem Solving
syllabus. It keeps a class list, lets an enrolled student check in with a
private four-digit PIN, prevents a second check-in for the same roll number
on the same date, and shows today's present and absent students.

SYLLABUS CONCEPTS USED
----------------------
* C program structure, standard headers, printf and fgets
* Variables, data types, operators, conditions, loops and switch
* Functions with parameters and return values
* Structures and arrays
* Strings and string-library functions
* Text-file input and output with fopen, fgets, fprintf and fclose

BUILD AND RUN
-------------
Open a terminal in this folder and compile with a C99 compiler:

    gcc -std=c99 -Wall -Wextra -pedantic attendance.c -o attendance

On Windows with MinGW, run:

    .\attendance.exe

The program creates students.txt and attendance.txt in its current folder.
Keep those files together with the program; they contain the saved class
list and attendance records.

HOW TO USE
----------
1. Choose "Enroll a student (instructor)" to add each student once.
2. Give each student their own roll number and private four-digit PIN.
3. Students choose "Student check-in" and enter their roll number and PIN.
4. Choose "View today's attendance" to see the present/absent list.

ANTI-PROXY NOTE
---------------
The roll-number/PIN check and one check-in per day help discourage proxy
attendance. A PIN can still be shared, so this classroom project cannot
prove a student's physical identity. For a stronger check, an instructor
should supervise check-in; real identity verification needs additional
systems beyond the C topics in this syllabus. The sample stores PINs as
plain text and is intended for a local learning project, not real student
records.
