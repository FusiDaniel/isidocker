/*
Features

isidocker pull <imagem>
    - faz um WGET da imagem em https://www.professorisidro.com.br/dockerimages/<imagem>.tar
    - descompacta em /home/user/.isidocker/images

isidocker run <imagem>
    - executa o comando que está disponível em <imagem>.isicommand

isidocker shell <imagem>
    - acessa a imagem e executa /bin/bash
*/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sched.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"
#define SEPARATE_LOGS_SEQUENCE "----------------------------------------\n"
#define STACK_SIZE 4096
#define MAX_LINE_LENGTH 1024

#define MODE_COMMAND 0
#define MODE_SHELL 1
#define MODE_SHELL2 2
#define MODE_RUN_DAEMON 3

int mode;
char image_dir[255];
char _image[255];
char log_file_dir[255];
char user_home[255] = "/home/opc";

// definindo os prototipos de funcoes
int childProcess(void *args);
int manage_container_commands(void);
int run_daemon(void);
int run_shell(char *daemon_pid_str);
int isFileEmpty(const char *filename);
int fileLineCount(const char *filename);
int get_daemon_child_pid(char *daemon_pid_str);
int run_java(char *daemon_pid_str);
int killDaemon(char *daemon_pid_str);

int main(int argc, char *argv[])
{
    printf(ANSI_COLOR_CYAN);
    printf("/***\n");
    printf("*    .___       .__________                 __\n");
    printf("*    |   | _____|__\\______ \\   ____   ____ |  | __ ___________\n");
    printf("*    |   |/  ___/  ||    |  \\ /  _ \\_/ ___\\|  |/ // __ \\_  __ \\\n");
    printf("*    |   |\\___ \\|  ||    `   (  <_> )  \\___|    <\\  ___/|  | \\/\n");
    printf("*    |___/____  >__/_______  /\\____/ \\___  >__|_ \\___  >__|\n");
    printf("*             \\/           \\/            \\/     \\/    \\/       \n");
    printf("*/\n\n");
    printf(ANSI_COLOR_YELLOW);
    printf("IsiDocker - Container Tool for Educational Purposes\n");
    printf("---------------------------------------------------\n");

    /* check if .isidocker_images folder exists. If doesn't, create it */
    strcpy(image_dir, user_home);
    strcat(image_dir, "/.isidocker_images/");
    struct stat s = {0};
    if (!stat(image_dir, &s))
    {
        printf(">> IsiDOCKER - Image Folder Exists\n");
    }
    else
    {
        printf(">> IsiDOCKER - Image Foder does not exist - Creating...\n");
        mkdir(image_dir, 0777);
    }
    printf(ANSI_COLOR_RESET);
    /* end of check */

    if (argc < 2)
    {

        printf(ANSI_COLOR_YELLOW);
        printf("Usage: isidocker COMMAND IMAGE_NAME\n");
        printf("Available Commands:\n");

        printf(" - pull <image_name>    : download and uncompress Image from Isidocker Repo\n");
        printf(" - run <image_name>     : starts a container\n");
        printf(" - shell <image_name>   : access the container and runs a shell\n");
        printf(" - extract <image_name> : uncompress Image from tar.gz file deleting existing one\n");
        printf(" - daemon <image_name>  : creates a daemon for the 'run' command\n");
        printf(" - log <image_name>     : show logs of the last run of a container\n");
        printf(" - log-all <image_name> : show logs of the last run of a container\n");
        printf(ANSI_COLOR_RESET);
        return 0;
    }

    char *_command = argv[1];
    strcpy(_image, argv[2]);
    pid_t pid, pid2;
    int status;
    int namespaces = CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWIPC | CLONE_NEWNS; // | CLONE_NEWNET;
    pid_t containerPid;

    if (!strcmp(_command, "pull"))
    {
        pid = fork();
        char cmd[255];
        strcpy(cmd, "https://www.professorisidro.com.br/images/");
        strcat(cmd, _image);
        strcat(cmd, ".tar.gz");
        if (pid == 0)
        {
            printf(ANSI_COLOR_YELLOW ">>> IsiDOCKER - Downloading image from %s\n" ANSI_COLOR_RESET, cmd);
            execl("/bin/wget", "wget", cmd, NULL);
        }
        else if (pid > 0)
        {
            waitpid(pid, &status, 0);
            printf(ANSI_COLOR_YELLOW "\n>>> IsiDOCKER - Image Download Complete!" ANSI_COLOR_RESET);
        }

        pid2 = fork();
        if (pid2 == 0)
        {
            char extrac_folder[255];
            strcpy(extrac_folder, image_dir);
            char zip_image[255];
            strcpy(zip_image, _image);
            strcat(zip_image, ".tar.gz");
            printf(ANSI_COLOR_YELLOW ">>> IsiDOCKER - Extracting image %s\n " ANSI_COLOR_RESET, _image);
            execl("/bin/tar", "tar", "-xf", zip_image, "-C", extrac_folder, NULL);
        }
        else if (pid2 > 0)
        {
            waitpid(pid2, &status, 0);
            printf(ANSI_COLOR_YELLOW "\n>>> IsiDOCKER - Image Extraction Complete!\n" ANSI_COLOR_RESET);
        }
    }
    else if (!strcmp(_command, "extract"))
    {
        pid = fork();
        if (pid == 0)
        {
            char extrac_folder[255];
            strcpy(extrac_folder, image_dir);
            char zip_image[255];
            strcpy(zip_image, _image);
            strcat(zip_image, ".tar.gz");
            printf(ANSI_COLOR_YELLOW ">>> IsiDOCKER - Extracting image %s\n " ANSI_COLOR_RESET, _image);
            execl("/bin/tar", "tar", "-xf", zip_image, "-C", extrac_folder, NULL);
        }
        else if (pid > 0)
        {
            waitpid(pid, &status, 0);
            printf(ANSI_COLOR_YELLOW "\n>>> IsiDOCKER - Image Extraction Complete!\n" ANSI_COLOR_RESET);
        }
    }
    else if (!strcmp(_command, "run"))
    {
        strcat(image_dir, _image);
        printf(ANSI_COLOR_YELLOW ">> IsiDOCKER - Running Image %s \n                 available on FileSystem %s\n" ANSI_COLOR_RESET, _image, image_dir);
        mode = MODE_COMMAND;
        containerPid = clone(childProcess, malloc(STACK_SIZE) + STACK_SIZE, SIGCHLD, NULL);
        if (containerPid == -1)
        {
            perror("clone");
            exit(1);
        }
        waitpid(containerPid, NULL, 0);
        return 0;
    }
    else if (!strcmp(_command, "shell"))
    {
        strcat(image_dir, _image);
        printf(ANSI_COLOR_YELLOW ">> IsiDOCKER - Running SHELL %s \n               available on FileSystem %s\n" ANSI_COLOR_RESET, _image, image_dir);
        mode = MODE_SHELL;
        containerPid = clone(childProcess, malloc(STACK_SIZE) + STACK_SIZE, SIGCHLD, NULL);
        if (containerPid == -1)
        {
            perror("clone");
            exit(1);
        }
        waitpid(containerPid, NULL, 0);
        return 0;
    }
    else if (!strcmp(_command, "daemon"))
    {
        strcat(image_dir, _image);

        char daemon_pid_str[255];
        if (!get_daemon_child_pid(daemon_pid_str)) // kill daemon if already running
        {
            printf(ANSI_COLOR_YELLOW ">> IsiDOCKER - Another Daemon Is Currently Running\n");
            printf(">> IsiDOCKER - Killing Old And Initializing New Daemon \n" ANSI_COLOR_RESET);
            killDaemon(daemon_pid_str);
        }
        else
            printf(ANSI_COLOR_YELLOW ">> IsiDOCKER - Initializing Daemon\n" ANSI_COLOR_RESET);
        mode = MODE_RUN_DAEMON;
        containerPid = clone(childProcess, malloc(STACK_SIZE) + STACK_SIZE, SIGCHLD | namespaces, NULL);
        if (containerPid == -1)
        {
            perror("clone");
            exit(1);
        }
        get_daemon_child_pid(daemon_pid_str);
        return 0;
    }
    else if (!strcmp(_command, "stop"))
    {
        strcat(image_dir, _image);
        char daemon_pid_str[255];
        if (!get_daemon_child_pid(daemon_pid_str)) // kill daemon if already running
        {
            printf(">> IsiDOCKER - Stopping Running Daemon \n" ANSI_COLOR_RESET);
            killDaemon(daemon_pid_str);
        }
        else
        {
            printf(ANSI_COLOR_YELLOW ">> IsiDOCKER - No Running Daemon Found\n" ANSI_COLOR_RESET);
        }
    }
    else if (!strcmp(_command, "log"))
    {
        strcat(image_dir, _image);
        strcpy(log_file_dir, image_dir);
        strcat(log_file_dir, "/logfile");
        system("clear");
        printf(ANSI_COLOR_YELLOW ">> IsiDOCKER - Reading logs from %s\n", log_file_dir);

        FILE *file = fopen(log_file_dir, "r");
        if (file == NULL)
        {
            perror("Error opening file");
            return 1;
        }
        fseek(file, 0, SEEK_END);

        char currentLine[MAX_LINE_LENGTH];
        long currentPosition = ftell(file);
        int lineNumber = 0;

        while (currentPosition > 0)
        {
            fseek(file, --currentPosition, SEEK_SET);
            char currentChar = fgetc(file);
            if (currentChar == '\n')
            {
                if (fgets(currentLine, MAX_LINE_LENGTH, file) != NULL)
                {
                    lineNumber++;
                    if (strcmp(currentLine, SEPARATE_LOGS_SEQUENCE) == 0)
                        break;
                }
            }
        }

        char firstLogLine[255];
        lineNumber = fileLineCount(log_file_dir) - lineNumber - 1;

        // if not empty, add 5 to the line number to skip the separator
        if (lineNumber)
            lineNumber += 5;

        sprintf(firstLogLine, "%d", lineNumber);
        char tail_command[255] = "tail -f -n +";
        strcat(tail_command, firstLogLine);
        strcat(tail_command, " ");
        strcat(tail_command, log_file_dir);
        system(tail_command);

        fclose(file);
    }
    else if (!strcmp(_command, "log-all"))
    {
        strcat(image_dir, _image);
        strcpy(log_file_dir, image_dir);
        strcat(log_file_dir, "/logfile");
        system("clear");
        printf(ANSI_COLOR_YELLOW ">> IsiDOCKER - Reading all logs from %s\n", log_file_dir);

        char tail_command[255] = "tail -f -n +0 ";
        strcat(tail_command, log_file_dir);
        system(tail_command);
    }
    else
    {
        printf("ERROR - Command: %s is not supported\n", _command);
    }
    return 0;
}

int killDaemon(char *daemon_pid_str)
{
    char kill_command[512];
    sprintf(kill_command, "kill -9 %s\n", daemon_pid_str);
    system(kill_command);

    char umount_proc_command[512];
    sprintf(umount_proc_command, "umount %s/proc", image_dir);
    system(umount_proc_command);
}

int get_daemon_child_pid(char *daemon_pid_str)
{
    FILE *fp = popen("lsns -t pid | grep \"javacontainer\" | awk '{print $4}'", "r");
    if (fp == NULL)
    {
        perror("Error opening pipe");
        exit(EXIT_FAILURE);
    }
    pid_t daemon_pid = 0;
    fscanf(fp, "%d", &daemon_pid);
    sprintf(daemon_pid_str, "%d", daemon_pid);
    if (daemon_pid == 0) // daemon is not running
        return 1;
    return 0;
}

int childProcess(void *args)
{
    (void)args;
    manage_container_commands();
    return (0);
}
int manage_container_commands(void)
{
    if (mode == MODE_RUN_DAEMON)
        run_daemon();

    char daemon_pid_str[255];
    if (get_daemon_child_pid(daemon_pid_str))
    {
        printf(ANSI_COLOR_YELLOW ">> IsiDOCKER - No daemon found, initialize daemon first with command: isidocker daemon <imagem>\n");
        return 1;
    }
    if (mode == MODE_COMMAND)
        run_java(daemon_pid_str);
    else
        run_shell(daemon_pid_str);
}

int fileLineCount(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        perror("Error opening file");
        return -1;
    }

    int lineCount = 0;
    int ch;

    while ((ch = fgetc(file)) != EOF)
    {
        if (ch == '\n')
        {
            lineCount++;
        }
    }

    fclose(file);
    return lineCount;
}
int isFileEmpty(const char *filename)
{
    FILE *file = fopen(filename, "rb");

    if (file == NULL || fseek(file, 0, SEEK_END) != 0)
    {
        perror("Error opening file");
        return -1;
    }

    long fileSize = ftell(file);
    fclose(file);
    return (fileSize == 0);
}

int run_daemon(void)
{
    chroot(image_dir);
    chdir("/");
    mount("proc", "proc", "proc", 0, "");
    while (1)
        pause();
}
int run_java(char *daemon_pid_str)
{
    char nsenter_command[1024];
    sprintf(nsenter_command, "nsenter --target %s --mount --uts --ipc --net --pid --root=\"%s\" --wd=\"%s\" bash -c '/jdk-20.0.2/bin/java -jar hello-0.0.1-SNAPSHOT.jar'", daemon_pid_str, image_dir, image_dir);
    printf("Entering container with PID: %s\n", daemon_pid_str);
    system(nsenter_command);
}
int run_shell(char *daemon_pid_str)
{
    char nsenter_command[1024];
    sprintf(nsenter_command, "nsenter --target %s --mount --uts --ipc --net --pid --root=\"%s\" --wd=\"%s\" bash", daemon_pid_str, image_dir, image_dir);
    printf("Entering container with PID: %s\n", daemon_pid_str);
    system(nsenter_command);
}