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

#define STACK_SIZE 4096

#define MODE_COMMAND 0
#define MODE_SHELL 1

int mode;
char image_dir[255];
char _image[255];
char user_home[255] = "/home/opc";

// definindo os prototipos de funcoes
int childProcess(void *args);
int create_container(void);
int run_container(void);
int run_shell(void);

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
        printf(" - extract <image_name> : uncompress Image from tar.gz file deleting existing one\n");
        printf(" - daemon <image_name>  : creates a daemon for the 'run' command\n");
        printf(" - run <image_name>     : starts a container\n");
        printf(" - shell <image_name>   : access the container and runs a shell\n");
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
        containerPid = clone(childProcess, malloc(STACK_SIZE) + STACK_SIZE, SIGCHLD | namespaces, NULL);
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
        containerPid = clone(childProcess, malloc(STACK_SIZE) + STACK_SIZE, SIGCHLD | namespaces, NULL);
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
        char service_file_path[255] = "/etc/systemd/system/isidocker-";
        strcat(service_file_path, _image);
        strcat(service_file_path, ".service");

        // remove file if exists
        remove(service_file_path);

        // create service file
        FILE *file = fopen(service_file_path, "w");

        if (file == NULL)
        {
            printf("ERROR - Could not create file %s\n", service_file_path);
        }

        fprintf(file, "[Unit]\n");
        fprintf(file, "Description=IsiDocker Daemon\n\n");
        fprintf(file, "[Service]\n");
        fprintf(file, "ExecStart=%s/isidocker.e run %s\n\n", user_home, _image);
        fprintf(file, "[Install]\n");
        fprintf(file, "WantedBy=multi-user.target\n");
        fclose(file);

        printf(">> IsiDOCKER - Daemon created at %s\n", service_file_path);

        /*  reloads systemd manager configuration,
            enables isidocker service (make it start on boot), and
            starts isidocker service (make it start now)*/

        char enable_command[255] = "systemctl enable isidocker-";
        strcat(enable_command, _image);
        strcat(enable_command, ".service");
        char start_command[255] = "systemctl start isidocker-";
        strcat(start_command, _image);
        strcat(start_command, ".service");

        system("systemctl daemon-reload");
        system(enable_command);
        system(start_command);

        printf(">> IsiDOCKER - Daemon enabled and started!\n");
    }
    else
    {
        printf("ERROR - Command: %s is not supported\n", _command);
    }
    return 0;
}

int childProcess(void *args)
{
    (void)args;
    create_container();
    return (0);
}

int create_container(void)
{
    if (mode == MODE_COMMAND)
    {
        run_container();
    }
    else
    {
        run_shell();
    }
}

int run_container(void)
{
    // Initialize logging to file
    char log_file_dir[255];
    strcpy(log_file_dir, image_dir);
    strcat(log_file_dir, "/logfile.txt");
    printf(ANSI_COLOR_YELLOW ">> IsiDOCKER - Creating log file at %s\n", log_file_dir);

    int logFile = open(log_file_dir, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (logFile == -1) {
        perror("Error opening log file");
        return 1;
    }
    if (dup2(logFile, STDOUT_FILENO) == -1 || dup2(logFile, STDERR_FILENO) == -1) {
        perror("Error redirecting output");
        return 1;
    }
    close(logFile);

    // Mount proc and run application
    chroot(image_dir);
    chdir("/");

    char *cmd[] = {"/jdk-20.0.2/bin/java", "-jar", "hello-0.0.1-SNAPSHOT.jar", NULL};
    mount("proc", "proc", "proc", 0, "");
    printf(">> IsiDOCKER - Proc filesystem mounted\n" ANSI_COLOR_RESET);
    execv("/jdk-20.0.2/bin/java", cmd);
    perror("exec");
    exit(EXIT_FAILURE);
}
int run_shell(void)
{
    char *cmd[] = {"/bin/bash", NULL};

    chroot(image_dir);
    printf(ANSI_COLOR_YELLOW ">> IsiDOCKER - Changing root directory to %s\n", image_dir);
    chdir("/");
    mount("proc", "proc", "proc", 0, "");
    printf(">> IsiDOCKER - Proc filesystem mounted\n" ANSI_COLOR_RESET);
    execv("/bin/bash", cmd);
    perror("exec");
    exit(EXIT_FAILURE);
}