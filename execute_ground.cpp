#include <iostream>
#include <sys/wait.h>
#include <unistd.h>
#include <libssh/libssh.h>
#include <stdlib.h>
#include <string.h>

using std::cout;
using std::endl;

int execute_commands(ssh_session session){
    ssh_channel channel;
    int rc;

    channel = ssh_channel_new(session);
    if (channel == NULL)
        return SSH_ERROR;

    rc = ssh_channel_open_session(channel);
    if (rc != SSH_OK)
    {
        ssh_channel_free(channel);
        return rc;
    }

    rc = ssh_channel_request_exec(channel, "cd /etc && ls | wc");
    if (rc != SSH_OK)
    {
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        return rc;
    }

    char buffer[256];
    int nbytes;

    nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
    while (nbytes > 0)
    {
        if (fwrite(buffer, 1, nbytes, stdout) != nbytes)
        {
            ssh_channel_close(channel);
            ssh_channel_free(channel);
            return SSH_ERROR;
        }
        nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
    }

    if (nbytes < 0)
    {
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        return SSH_ERROR;
    }

    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);

    return SSH_OK;
}

int execute_child(int child_id){
    ssh_session my_ssh_session;
    int rc;
    char password[] = "password";
    char connection_string[] = "localhost";

    my_ssh_session = ssh_new();
    if (my_ssh_session == NULL)
        return 1;

    ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, connection_string);

    cout << "SSH connecting to: " << connection_string << " for child " << child_id << endl;
    rc = ssh_connect(my_ssh_session);
    if (rc != SSH_OK)
    {
        fprintf(stderr, "Error connecting to localhost: %s\n",
                ssh_get_error(my_ssh_session));
        ssh_free(my_ssh_session);
        return 1;
    }

    cout << "SSH connection succes, authenticating child " << child_id << endl;

    rc = ssh_userauth_password(my_ssh_session, NULL, password);
    if (rc != SSH_AUTH_SUCCESS)
    {
        fprintf(stderr, "Error authenticating with password: %s\n",
                ssh_get_error(my_ssh_session));
        ssh_disconnect(my_ssh_session);
        ssh_free(my_ssh_session);
        return 1;
    }

    cout << "Authentication success for child " << child_id << endl;

    rc = execute_commands(my_ssh_session);

    ssh_disconnect(my_ssh_session);
    ssh_free(my_ssh_session);

    cout << "SSH session " << child_id << " closed" << endl;
}

int main(){
    int NUM_CHILDREN = 4;

    pid_t children_pid[NUM_CHILDREN];
    for (int i = 0; i < NUM_CHILDREN; i++)
    {
        pid_t pid = fork();
        if (pid == -1)
        {
            perror("Error creating child");
            return 1;
        }
        if (pid == 0)
        {
            cout << "Child " << i << " created" << endl;
            execute_child(i);
            return 0;
        }
        children_pid[i] = pid;
    }

    for (int i = 0; i < NUM_CHILDREN; i++)
    {
        wait(nullptr);
        cout << "Acknowledged " << i + 1 << " child return" << endl;
    }
}