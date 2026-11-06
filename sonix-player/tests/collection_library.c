#include "src/system/library/library.h"
#include "src/system/library/playlists.h"
#include "src/system/library/collections.h"
#include "src/system/core/config.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
int main(int argc,char **argv){assert(argc==2);char db[768],cfg[768],path[512];
 snprintf(db,sizeof(db),"%s/.local/library.db",argv[1]);snprintf(cfg,sizeof(cfg),"%s/config.ini",argv[1]);config_init(cfg);assert(library_open(db));playlists_init(argv[1]);collections_init(argv[1]);
 for(int i=0;i<5;i++){snprintf(path,sizeof(path),"%s/Music/track%d.wav",argv[1],i);assert(library_import_file(path));for(int j=0;library_scan_running()&&j<2000;j++)usleep(10000);assert(!library_scan_running());}
 snprintf(path,sizeof(path),"%s/Music/track0.wav",argv[1]);char names[16][256];int n=library_track_artists(path,names,16);assert(n==4);for(int i=0;i<n;i++)assert(!strchr(names[i],',')&&!strstr(names[i]," - "));
 library_release_t releases[12];assert(library_discography("Herta FLEBBE",0,0,12,releases)>0);
 library_playlist_row_t songs[3];assert(library_song_search_page("",0,3,songs)==3);assert(library_song_search_page("",3,3,songs)==2);assert(library_song_search_page("Night Walk",0,3,songs)==1);
 assert(collections_catalog_sync());collection_item_t rows[48];assert(collections_browse(2,COLLECTION_NAME,"","",0,rows,48)>0);assert(!rows[0].saved);
 assert(collections_browse(3,COLLECTION_NAME,"herta","",0,rows,48)==1);assert(!strcmp(rows[0].title,"Herta FLEBBE"));
 assert(playlists_create("Road Mix"));assert(playlists_add_track("Road Mix",path));collections_put(COLLECTION_PLAYLIST,"Road Mix","Road Mix","You",true);
 assert(collections_folder_create("Driving"));assert(!collections_folder_create("Driving"));assert(collections_move_playlist("Road Mix","Driving"));assert(collections_folder_count("Driving")==1);
 assert(collections_browse(1,COLLECTION_NAME,"","",0,rows,48)==1 && rows[0].kind==COLLECTION_FOLDER);
 assert(collections_browse(0,COLLECTION_NAME,"","Driving",0,rows,48)==1 && !strcmp(rows[0].key,"Road Mix"));
 assert(collections_browse(0,COLLECTION_NAME,"road","",0,rows,48)==1);
 assert(collections_rename(COLLECTION_FOLDER,"Driving","Trips","Trips"));assert(collections_folder_count("Trips")==1);
 assert(playlists_rename("Road Mix","Travel Mix"));assert(collections_rename(COLLECTION_PLAYLIST,"Road Mix","Travel Mix","Travel Mix"));assert(collections_browse(0,COLLECTION_NAME,"","Trips",0,rows,48)==1 && !strcmp(rows[0].key,"Travel Mix"));
 assert(collections_pin(COLLECTION_FOLDER,"Trips",true));assert(collections_browse(0,COLLECTION_NAME,"","",0,rows,48)>0 && !strcmp(rows[0].key,"Trips"));
 assert(collections_forget(COLLECTION_FOLDER,"Trips"));assert(playlists_has_track("Travel Mix",path));assert(collections_browse(1,COLLECTION_NAME,"","",0,rows,48)==1);
 assert(collections_custom_first(COLLECTION_PLAYLIST,"Travel Mix"));assert(collections_browse(0,COLLECTION_CUSTOM,"","",0,rows,48)>0 && !strcmp(rows[0].key,"Travel Mix"));
 assert(collections_folder_create("Keep"));assert(collections_move_playlist("Travel Mix","Keep"));collections_init(argv[1]);assert(collections_folder_count("Keep")==1);
 assert(playlists_delete("Travel Mix"));assert(collections_forget(COLLECTION_PLAYLIST,"Travel Mix"));assert(collections_folder_count("Keep")==0);assert(!access(path,R_OK));

 /* Mixed case and accents share one ordered alphabet, including the grip.
  * Page through the same query used by Your Library, not a test-side sort. */
 const char *mixed[]={"zulu","Bravo","alpha","\xc3\x81lpha","charlie","The Delta"};
 const char *ordered[]={"alpha","\xc3\x81lpha","Bravo","charlie","The Delta","zulu"};
 for(int i=0;i<6;i++) assert(collections_put(COLLECTION_ARTIST,mixed[i],mixed[i],"Case fixture",true));
 for(int i=0;i<6;i++) {
   assert(collections_browse(3,COLLECTION_NAME,"Case fixture","",i,rows,1)==1);
   assert(!strcmp(rows[0].title,ordered[i]));
 }
 assert(library_index_letter("alpha")==library_index_letter("Alpha"));
 assert(library_index_letter("\xc3\x81lpha")=='A');
 assert(library_index_letter("The Delta")=='D');
 assert(playlists_create("Find test"));
 for(int i=0;i<3;i++){snprintf(path,sizeof(path),"%s/Music/track%d.wav",argv[1],i);assert(playlists_add_track("Find test",path));}
 library_index_t *base=library_index_open(LIBRARY_LIST_PLAYLIST,LIBRARY_FILTER_NONE,"Find test",LIBRARY_ORDER_DEFAULT,false);assert(library_index_count(base)==3);
 library_index_t *v=library_index_find_view(base,"cafe",0);assert(v && library_index_count(v)==1);assert(library_index_source_position(v,0,base)==1);library_index_close(v);
 v=library_index_find_view(base,"Night Lights",0);assert(v && library_index_count(v)==3);library_index_close(v);
 v=library_index_find_view(base,"Herta",0);assert(v && library_index_count(v)==1);library_index_close(v);
 v=library_index_find_view(base,"First Light",0);assert(v && !library_index_count(v));library_index_close(v);
 v=library_index_find_view(base,"%",0);assert(v && !library_index_count(v));library_index_close(v);
 v=library_index_find_view(base,"",1);assert(v && library_index_count(v)==3);assert(library_index_source_position(v,0,base)==2);library_index_close(v);
 for(int sort=2;sort<5;sort++){v=library_index_find_view(base,"",sort);assert(v && library_index_count(v)==3);library_index_close(v);}
 assert(library_index_count(base)==3);library_index_close(base);
 library_close();puts("PASS collection library: individual role credits and discography, unsaved indexed albums/artists, filtered search, one-level folders, playlist/folder renames, pinned order, restart persistence, deletion preserving audio");}
