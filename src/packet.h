#ifndef _PACKET_H_
#define _PACKET_H_

#include <stdarg.h>
#include <stdio.h>

#include "connection.h"
#include "packet_data.h"

class packet
{
	public:
		packet(connection *serve, sdl_user *blabla);
		~packet();
		
		void send_packet(packet_data &sendme);
		void send_packet(const char* args, ...);
		void send_packet(const char* args, va_list array);
		packet_data &get_packet(bool translate);
		void print_packet(uint8_t opcode, packet_data &printme, const char *msg);

		int process_packet();
	private:
		char decryptionKey[8];
		char encryptionKey[8];
		int mode;	//what mode is packet decoding in?
		volatile int key_initialized;
		volatile int modern_protocol;
		unsigned int modern_read_total;
		unsigned int modern_write_total;
		int modern_login_sent;
		int modern_enter_sent;
		int modern_char_count;
		int modern_chars_seen;
		char modern_account[64];
		char modern_password[64];
		char modern_char_name[64];
		connection *server;
		sdl_user *theuser;
		packet_data data;

		void encrypt(packet_data &eme);
		void decrypt(packet_data &dme);
		void modern_encrypt(unsigned char *body, int len);
		void modern_decrypt(unsigned char *body, int len);
		void modern_send_login();
		void modern_send_enter_world(const char *char_name);
		void modern_object_add();
		void change_key(char *key, const char *data);	//changes the encryption key
		void create_key(const unsigned int seed);
		
		//packet handlers
		void key_packet();
		void ac_and_elemental_update();
		void char_status();
		void char_title();
		void char_alignment();
		void dex_update();
		void move_object();
		void change_heading();
		void remove_object();
		void str_update();
		void set_map();
		void server_version_packet();
		void news_packet();
		void num_char_packet();
		void login_char_packet();
		void login_check();
		void char_create_result();
		void handle_chat(unsigned char opcode);
		void game_time();
		void update_mp();
		void update_hp();
		void ground_item();
		void place_light();
		void change_spmr();
		void weather();
		void add_inv_items();
		void item_bless_status();
		void server_message();
};

#endif
