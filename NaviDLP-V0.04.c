// NaviDLP, for Navidrome and YT-DLP Integration including Search and Download.
//
// INFO:
// This Project Is Intended For PERSONAL USE ONLY.
// PORT: 6969
// CURRENT VERSION: V0.04 (INCOMPLETE) (COMPLETED DLOADSYS)
//
// USE DEPENDENCIES:
// cjson
// libmicrohttpd
// curl
// yt-dlp
//
// GLOBAL TODO: FINISH SEARCHSYS (NAVIDROME RESULT COMBINATION) MAKE IDLE STATE
// GLOBAL PLAN:
// ESTIMATE TO STABLE: 82%

#include <stdio.h>
#include <stdlib.h>
#include <microhttpd.h>
#include <cjson/cJSON.h>
#include <curl/curl.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define	NOOB 6969
#define NAVIURL "http://localhost:4533"

// CURL PREREQUISITES (i really need to learn this one but fuck it i need something shippable.)
struct membuf { char *data; size_t size; };
static size_t writecb(void *contents, size_t size, size_t nmemb, void *userp){
    struct membuf *buf = userp;
    buf->data = realloc(buf->data, buf->size + size*nmemb + 1);
    memcpy(buf->data + buf->size, contents, size*nmemb);
    buf->size += size*nmemb;
    buf->data[buf->size] = 0;
    return size*nmemb;
}
//
// FORWARD DECLARATIONS
static enum MHD_Result catcher(void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, size_t *upload_data_size, void **con_cls);
static enum MHD_Result searchsys(struct MHD_Connection *connection);
static enum MHD_Result searchsys(struct MHD_Connection *connection);

// LISTENER DAEMON
int main (){ //supposed to listenign yes
 struct MHD_Daemon *listenign = MHD_start_daemon(MHD_USE_INTERNAL_POLLING_THREAD,NOOB,NULL,NULL,&catcher,NULL,MHD_OPTION_END);
	if(listenign==NULL) return 1;
	getchar();
	MHD_stop_daemon(listenign);
	return 0;
}

// INTERCEPTION SYSTEM (CATCHER)
static enum MHD_Result catcher(void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, size_t *upload_data_size, void **con_cls){
if(strcmp(url,"/rest/search3.view")==0){
	return searchsys(connection);
} else if(strcmp(url,"/rest/stream.view")==0){
	return dloadsys(connection);
}
//else 
	//return handle_proxy (connection);
}

// SEARCH SYSTEM
// TODO: ADD COMBINE FOR NAVIDROME RESULTS.
// PLAN:
//
static enum MHD_Result searchsys(struct MHD_Connection *connection){ // SEARCH SYSTEM START
char ytjson[69420];
// GET QUERY AND OTHER ASSEMBLY PREREQUISITES
const char *query = MHD_lookup_connection_value(connection,MHD_GET_ARGUMENT_KIND,"query");
const char *user = MHD_lookup_connection_value(connection,MHD_GET_ARGUMENT_KIND,"u");
const char *token = MHD_lookup_connection_value(connection,MHD_GET_ARGUMENT_KIND,"t");
const char *salt = MHD_lookup_connection_value(connection,MHD_GET_ARGUMENT_KIND,"s");
const char *clientname = MHD_lookup_connection_value(connection,MHD_GET_ARGUMENT_KIND,"c");
const char *apiversion = MHD_lookup_connection_value(connection,MHD_GET_ARGUMENT_KIND,"v");
	
// FORWARD TO NAVIDROME
char searchquery [1337];
snprintf(searchquery,sizeof(searchquery),"http://localhost:4533/rest/search3.view?query=%s&u=%s&t=%s&s=%s&c=%s&v=%s",query,user,token,salt,clientname,apiversion);
struct membuf naviresp = {0};
CURL *navisearch = curl_easy_init();
	curl_easy_setopt(navisearch, CURLOPT_WRITEFUNCTION, writecb);
	curl_easy_setopt(navisearch, CURLOPT_WRITEDATA, &naviresp);
	curl_easy_setopt(navisearch,CURLOPT_URL,searchquery);
	curl_easy_perform(navisearch);
	curl_easy_cleanup(navisearch);
	
// YT-DLP SEARCH EXECUTION
int pipefds [2];
	pipe(pipefds);
	
pid_t pid=fork();
if(pid==0){
	dup2(pipefds[1], STDOUT_FILENO);
		close(pipefds[0]);
		close(pipefds[1]);
		
	char searchterm[420];
	snprintf(searchterm,sizeof(searchterm),"ytsearch5:%s",query);
		char *search[]={"yt-dlp",searchterm,"--dump-json","--no-download", NULL};
		execvp("yt-dlp",search);
}
	else{
		close(pipefds[1]);
		wait(NULL);
		read(pipefds[0],ytjson,sizeof(ytjson));
	}

// PARSE AND READ PIPE OUTPUT
	cJSON *songarray = cJSON_CreateArray();
	char *line = strtok(ytjson,"\n");
		while(line!= NULL){
		cJSON *ytread = cJSON_Parse(line);
		if(ytread == NULL) continue; // ytread ERROR HANDLER
			cJSON *musicname = cJSON_GetObjectItem(ytread,"title");
			cJSON *artist = cJSON_GetObjectItem(ytread,"uploader");
			cJSON *time = cJSON_GetObjectItem(ytread,"duration");
			cJSON *ytid = cJSON_GetObjectItem(ytread,"id");
			//cJSON *picture = cJSON_GetObjectItem(ytread,"thumbnail") DISABLED DUE TO INCOMPLETE IMPLEMENTATION
		if(ytid == NULL || musicname == NULL || artist == NULL || time == NULL) continue;

// WRITE TO API RETURN
	cJSON *ytresults = cJSON_CreateObject();
	char fullid[64];
		snprintf(fullid, sizeof(fullid), "YT-%s", ytid->valuestring);
		cJSON_AddStringToObject(ytresults, "id", fullid);
	    cJSON_AddStringToObject(ytresults,"title",musicname->valuestring);
	    cJSON_AddStringToObject(ytresults,"artist",artist->valuestring);
	    cJSON_AddNumberToObject(ytresults,"duration",time->valuedouble);
		cJSON_AddBoolToObject(ytresults, "isDir", 0);
		// ARRAY IT
		cJSON_AddItemToArray(songarray, ytresults);
	cJSON_Delete(ytread);
	line=strtok(NULL,"\n");
	}

// COMPILE TO CLEAN FORMATTED JSON
	cJSON *root = cJSON_CreateObject();
	cJSON *searchresult = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "status", "ok");
	cJSON_AddStringToObject(root, "version", "1.16.1");
	cJSON_AddItemToObject(searchresult, "song", songarray);
	cJSON_AddItemToObject(root, "searchResult3", searchresult);
	char *response = cJSON_Print(root);

// SENDS RESPONSE TO CLIENT
struct MHD_Response *searchresp = MHD_create_response_from_buffer(strlen(response), response, MHD_RESPMEM_MUST_FREE);
	MHD_add_response_header(searchresp, "Content-Type", "application/json");
	MHD_queue_response(connection, MHD_HTTP_OK, searchresp);
	MHD_destroy_response(searchresp);
cJSON_Delete(root);
return MHD_YES;
}

// DOWNLOAD SYSTEM
// TODO:
// PLAN:
//
static enum MHD_Result dloadsys(struct MHD_Connection *connection){
char title [69420];
// POINTER THE ARGUMENTS
const char *id = MHD_lookup_connection_value(connection,MHD_GET_ARGUMENT_KIND,"id");
// const char *title = MHD_lookup_connection_value(connection,MHD_GET_ARGUMENT_KIND,"title") ohio unsure?
//RECONSTRUCTION NESSECARY COMPONENTS
const char *user = MHD_lookup_connection_value(connection,MHD_GET_ARGUMENT_KIND,"u");
const char *token = MHD_lookup_connection_value(connection,MHD_GET_ARGUMENT_KIND,"t");
const char *salt = MHD_lookup_connection_value(connection,MHD_GET_ARGUMENT_KIND,"s");
const char *clientname = MHD_lookup_connection_value(connection,MHD_GET_ARGUMENT_KIND,"c");
const char *apiversion = MHD_lookup_connection_value(connection,MHD_GET_ARGUMENT_KIND,"v");
char streamreply[1337];
snprintf(streamreply, sizeof(streamreply),"http://localhost:4533/rest/stream.view?id=%s&u=%s&t=%s&s=%s&c=%s&v=%s&maxBitRate=128&format=mp3",id,user,token,salt,clientname,apiversion);

// CHECKS IF ITS ALREADY AVAIL ON NAVIDROME, IF YES THEN FORWARDS TO CLIENT
	if(strncmp(id,"YT-",3)!=0){
		struct membuf availstream = {0};
			CURL *availstreamer = curl_easy_init();
    		curl_easy_setopt(finalsender,CURLOPT_WRITEFUNCTION,writecb);
    		curl_easy_setopt(finalsender,CURLOPT_WRITEDATA,&availstream);
			curl_easy_setopt(availstreamer,CURLOPT_URL,streamreply);
			curl_easy_perform(availstreamer);
			curl_easy_cleanup(availstreamer);
// SEND STREAM
	struct MHD_Response *availstreamresp = MHD_create_response_from_buffer(streamresp.size,streamresp.data,MHD_RESPMEM_MUST_FREE);
		MHD_add_response_header(availstreamresp,"Content-Type","audio/mpeg");
		MHD_queue_response(connection,MHD_HTTP_OK,availstreamresp);
		MHD_destroy_response(availstreamresp);
		return MHD_YES;
	}

// DOWNLOAD EXECUTION AND TITLE RETRIEVAL
	else{
		int pipefds [2];
		pipe(pipefds);
	
		pid_t pid=fork();
		if(pid==0){
			dup2(pipefds[1], STDOUT_FILENO);
			close(pipefds[0]);
			close(pipefds[1]);
				char url[420];
				snprintf(url,sizeof(url),"https://www.youtube.com/watch?v=%s",id+3); // INCOMPLETE
				char *dload[]={"yt-dlp",url,"-x","--audio-format","mp3","--print","title","-o","/mnt/hdd/music/%(title)s.%(ext)s",NULL};
					execvp("yt-dlp",dload);
		}
		close(pipefds[1]);
		wait(NULL);
		read(pipefds[0],title,sizeof(title));
		title[strcspn(title, "\n")] = 0;

// RESCAN LIBRARY TO ENSURE IT IS SCANNED
CURL *naviscan = curl_easy_init();
	curl_easy_setopt(naviscan,CURLOPT_URL,"http://localhost:4533/rest/startScan");
	curl_easy_perform(naviscan);
	curl_easy_cleanup(naviscan);

// FIND THE MUSIC ID
char finalresults [1337];
snprintf(finalresults,sizeof(finalresults),"http://localhost:4533/rest/search3.view?query=%s&u=%s&t=%s&s=%s&c=%s&v=%s",title,user,token,salt,clientname,apiversion);
struct membuf naviresp = {0};
CURL *navisearchs = curl_easy_init();
	curl_easy_setopt(navisearchs, CURLOPT_WRITEFUNCTION, writecb);
	curl_easy_setopt(navisearchs, CURLOPT_WRITEDATA, &naviresp);
	curl_easy_setopt(navisearchs,CURLOPT_URL,finalresults);
	curl_easy_perform(navisearchs);
	curl_easy_cleanup(navisearchs);
	free(naviresp.data)
	if(getid==NULL) return MHD_NO;

// BUILD THE STREAM RESPONSE
	cJSON *getid = cJSON_Parse(naviresp.data);
	cJSON *subres = cJSON_GetObjectItem(getid, "subsonic-response");
	cJSON *searchresult = cJSON_GetObjectItem(subres, "searchResult3");
	cJSON *songarray = cJSON_GetObjectItem(searchresult, "song");
	cJSON *firstsong = cJSON_GetArrayItem(songarray, 0);
	cJSON *NVID = cJSON_GetObjectItem(firstsong, "id");
	if(subres==NULL || searchresult==NULL || songarray==NULL || firstsong==NULL || NVID==NULL) return MHD_NO;

// CALL NAVIDROME TO STREAM
char finalstream [1337];
snprintf(finalstream, sizeof(finalstream),"http://localhost:4533/rest/stream.view?id=%s&u=%s&t=%s&s=%s&c=%s&v=%s&maxBitRate=128&format=mp3",NVID->valuestring,user,token,salt,clientname,apiversion);
cJSON_Delete(getid);
struct membuf streamresp = {0};
CURL *finalsender = curl_easy_init();
    curl_easy_setopt(finalsender, CURLOPT_WRITEFUNCTION, writecb);
    curl_easy_setopt(finalsender, CURLOPT_WRITEDATA, &streamresp);
		curl_easy_setopt(finalsender,CURLOPT_URL,finalstream);
		curl_easy_perform(finalsender);
		curl_easy_cleanup(finalsender);

// SEND STREAM
struct MHD_Response *proxyresp = MHD_create_response_from_buffer(streamresp.size, streamresp.data, MHD_RESPMEM_MUST_FREE);
MHD_add_response_header(proxyresp, "Content-Type", "audio/mpeg");
MHD_queue_response(connection, MHD_HTTP_OK, proxyresp);
MHD_destroy_response(proxyresp);
return MHD_YES;
	}
}

// PROXY SYSTEM
// TODO: write it...
// PLAN: 
//
static enum MHD_Result proxy(struct MHD_Connection *connection){
