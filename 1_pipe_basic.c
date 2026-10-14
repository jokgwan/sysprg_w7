/* 부모가 문자열을 보내면 자식이 대문자로 바꿔 부모에게 돌려준다. */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "사용법: %s \"문자열\"\n", argv[0]);
        return 1;
    }
    int to_child[2], to_parent[2];
    if (pipe(to_child) == -1 || pipe(to_parent) == -1) {
        perror("pipe"); return 1;
    }
    pid_t pid = fork();
    if (pid == -1) { perror("fork"); return 1; }
    if (pid == 0) {
        close(to_child[1]); close(to_parent[0]);
        char buf[4096];
        ssize_t n = read(to_child[0], buf, sizeof(buf)-1);
        if (n < 0) { perror("read"); _exit(1); }
        buf[n] = '\0';
        for (ssize_t i=0; i<n; i++) buf[i] = (char)toupper((unsigned char)buf[i]);
        if (write(to_parent[1], buf, (size_t)n) < 0) { perror("write"); _exit(1); }
        close(to_child[0]); close(to_parent[1]); _exit(0);
    }
    close(to_child[0]); close(to_parent[1]);
    size_t len = strlen(argv[1]);
    if (write(to_child[1], argv[1], len) < 0) { perror("write"); return 1; }
    close(to_child[1]); /* EOF 전달 */
    char result[4096];
    ssize_t n = read(to_parent[0], result, sizeof(result)-1);
    if (n < 0) { perror("read"); return 1; }
    result[n] = '\0';
    close(to_parent[0]);
    waitpid(pid, NULL, 0);
    printf("[부모] 원본: %s\n[부모] 자식이 돌려준 대문자: %s\n", argv[1], result);
    return 0;
}
