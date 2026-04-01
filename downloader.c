// dhdhdhehjxjejdbdhhehdhgdgdhdgdhhsnehdhrd

#include stdio.h
#include stdlib.h
#include sysocket.h
#include netinet.h
#include arpainet.h

int main (){

char search, result, pass, download
int 

if(URLREQ=="/rest/search3.view"){
	send(NAVIDROME(/rest/search3))
// PIPEFDS: [1]=WRITE [2]=READ
	int pipefds [2];
     pipe(pipefds);
	
	pid_t pid=fork();
	if(pid=0){
		dup2(pipefds[1], STDOUT_FILENO);
			close(pipefds[0]);
			close(pipefds[1]);
		exec("yt-dlp ytsearch5 <query> --dump-json --no-download");
	} else{
		close(pipefds[1]);
		wait(NULL);
		read(pipefds[0]);
	}
	
	read(yt-dlp JSON, NAVIDROME JSON);
	if(result==yt-dlp JSON);
	printf("YT" before "musicname");
	combine;
	//send result to subsonic API



return 0;
}