#ifndef _PHONE_H_
#define _PHONE_H_

#define NO_PHONE	-1

#define PT_CIPHONE	0		/* Context independent */
#define PT_CDPHONE	-1		/* Context dependent */
#define PT_WWPHONE	-2		/* With in Word */
#define PT_DIPHONE	-3		/* DiPhone */
#define PT_DIPHONE_S	-4		/* DiPhone Singleton */
#define PT_CDDPHONE	-99		/* Context dependent duration */
#define PT_WWCPHONE	1000		/* With in Word Component phone */

int phone_read(char *filename);
void phone_add_diphones(void);

/* TODO: 'extern inline' most of these if GNU C or C99 is in effect */
int32 phone_to_id(char const *phone_str, int verbose);
char const *phone_from_id(int32 phone_id);
int32 phone_id_to_base_id(int32 phone_id);
int32 phone_type(int32 phone_id);
int32 phone_len(int32 phone_id);
int32 phone_count(void);
int32 phoneCiCount (void);
int32 phoneWdCount (void);
int32 phone_map (int32 pid);
list_t *phoneList (void);


#endif /* _PHONE_H_ */
