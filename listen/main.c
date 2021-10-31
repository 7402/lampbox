#include <stdio.h>
#include <sys/ioctl.h>
#include <stdbool.h>
#include <unistd.h>
#include <julius/juliuslib.h>

static void cb_event_poll(Recog *recog, void *data);
static void cb_event_speech_ready(Recog *recog, void *data);
static void cb_event_speech_start(Recog *recog, void *data);
static void cb_result(Recog *recog, void *dummy);

bool open_and_run = true;
bool wait_to_resume = false;

int main(int argc, char * argv[])
{
    Jconf *jconf;
    
    jlog_set_output(NULL);

    if (argc == 1) {
        jconf = j_config_load_file_new("listen.jconf");
        
    } else {
        jconf = j_config_load_args_new(argc, argv);
    }
    
    if (jconf == NULL) {
        fprintf(stderr, "No configuration.\n");
        return -1;
    }
    
    Recog *recog = j_create_instance_from_jconf(jconf);
    if (recog == NULL) {
        fprintf(stderr, "Bad startup.\n");
        return -1;
    }

    callback_add(recog, CALLBACK_EVENT_SPEECH_READY, cb_event_speech_ready, NULL);
    callback_add(recog, CALLBACK_EVENT_SPEECH_START, cb_event_speech_start, NULL);
    callback_add(recog, CALLBACK_RESULT, cb_result, NULL);
    callback_add(recog, CALLBACK_POLL, cb_event_poll, NULL);

    if (j_adin_init(recog) == 0) {
        fprintf(stderr, "Bad audio.\n");
        return -1;
    }
    
    while (open_and_run || wait_to_resume) {
        if (open_and_run) {
            if (j_open_stream(recog, NULL) != 0) {
                fprintf(stderr, "Bad input stream.\n");
                return -1;
            }

            int status = j_recognize_stream(recog);
            if (status == -1){
                fprintf(stderr, "Bad loop termination.\n");
                return -1;
            } else {
                fprintf(stderr, "j_recognize_stream status = %d\n", status);
            }
            
            open_and_run = false;
            
        } else if (wait_to_resume) {
            char c;
            int available = 0;
            ioctl(STDIN_FILENO, FIONREAD, &available);
            if (available > 0 && 1 == read(STDIN_FILENO, &c, 1)) {
                switch(c) {
                    case 'r':
                        open_and_run = true;
                        wait_to_resume = false;
                        fprintf(stderr, "resume listen\n");
                        break;
                    case 'x':
                        wait_to_resume = false;
                        fprintf(stderr, "exit listen\n");
                       break;
                    case '\n':
                        break;
                    default:
                        fprintf(stderr, "got '%c'\n", c);
                }
            }
        }
    }

    j_close_stream(recog);
    j_recog_free(recog);

    return(0);
}

static void cb_event_poll(Recog *recog, void *data)
{
    char c;
    int available = 0;
    ioctl(STDIN_FILENO, FIONREAD, &available);
    if (available > 0 && 1 == read(STDIN_FILENO, &c, 1)) {
        switch(c) {
            case 'p':
                wait_to_resume = true;
                fprintf(stderr, "p: j_close_stream\n");
                j_close_stream(recog);
                break;
            case 'x':
                fprintf(stderr, "x: j_close_stream\n");
                j_close_stream(recog);
            case '\n':
                break;
            default:
                fprintf(stderr, "got '%c'\n", c);
        }
    }

}

static void cb_event_speech_ready(Recog *recog, void *data)
{
    //fprintf(stderr, "ry>\n");
    printf(">\n");
    fflush(stdout);
}

static void cb_event_speech_start(Recog *recog, void *data)
{
    //fprintf(stderr, "st<\n");
}

static void cb_result(Recog *recog, void *dummy)
{
    //fprintf(stderr, "rs[\n");
    bool found_one = false;
    for (RecogProcess *process=recog->process_list; process != NULL; process = process->next) {
        if (process->live && process->result.status >= 0) {
            WORD_INFO *winfo = process->lm->winfo;

            for (int k = 0; k < process->result.sentnum; k++) {
                Sentence *sentence = &(process->result.sent[k]);
 
                WORD_ID *sequence = sentence->word;
                int sequence_count = sentence->word_num;
                LOGPROB this_min_confidence = 1.0;

                for (int i = 0; i < sequence_count; i++) {
                    if (this_min_confidence > sentence->confidence[i]) {
                        this_min_confidence = sentence->confidence[i];
                    }
                }
                
                printf("%4.2f", this_min_confidence);
                for (int i = 0; i < sequence_count; i++) {
                    char * woutput = winfo->woutput[sequence[i]];
                    if (woutput[0] != '<') printf(" %s", woutput);
                    //else fprintf(stderr, " %s", woutput);
                }
                printf("\n");
                found_one = true;
            }
        }
    }
    
    if (!found_one) printf("*\n");

    fflush(stdout);
}
