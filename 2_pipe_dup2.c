/* ./2_pipeline ls -l : wc -l 형태의 두 명령 파이프라인 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    int sep = -1;
    for (int i=1; i<argc; i++) if (strcmp(argv[i], ":") == 0) { sep=i; break; }
    if (sep <= 1 || sep == argc-1) {
        fprintf(stderr, "사용법: %s 명령1 [인자...] : 명령2 [인자...]\n", argv[0]);
        return 1;
    }
    argv[sep] = NULL;
    char **cmd1 = &argv[1], **cmd2 = &argv[sep+1];
    int fd[2];
    if (pipe(fd) == -1) { perror("pipe"); return 1; }
    pid_t p1=fork();
    if (p1 == -1) { perror("fork"); return 1; }
    if (p1 == 0) {
        close(fd[0]);
        if (dup2(fd[1], STDOUT_FILENO) == -1) { perror("dup2"); _exit(1); }
        close(fd[1]); execvp(cmd1[0], cmd1); perror("execvp 명령1"); _exit(127);
    }
    pid_t p2=fork();
    if (p2 == -1) { perror("fork"); close(fd[0]); close(fd[1]); waitpid(p1,NULL,0); return 1; }
    if (p2 == 0) {
        close(fd[1]);
        if (dup2(fd[0], STDIN_FILENO) == -1) { perror("dup2"); _exit(1); }
        close(fd[0]); execvp(cmd2[0], cmd2); perror("execvp 명령2"); _exit(127);
    }
    close(fd[0]); close(fd[1]); /* 부모가 쓰기 끝을 열어 두면 wc가 EOF를 받지 못함 */
    int s1,s2; waitpid(p1,&s1,0); waitpid(p2,&s2,0);
    return (WIFEXITED(s1) && WEXITSTATUS(s1)==0 && WIFEXITED(s2) && WEXITSTATUS(s2)==0) ? 0 : 1;
}
