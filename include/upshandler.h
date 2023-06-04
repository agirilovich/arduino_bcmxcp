enum {
	STAT_INSTCMD_HANDLED = 0,	/* completed successfully */
	STAT_INSTCMD_UNKNOWN,		/* unspecified error */
	STAT_INSTCMD_INVALID,		/* invalid command */
	STAT_INSTCMD_FAILED		/* command failed */
};

/* return values for setvar */
enum {
	STAT_SET_HANDLED = 0,	/* completed successfully */
	STAT_SET_UNKNOWN,	/* unspecified error */
	STAT_SET_INVALID,	/* not writeable */
	STAT_SET_FAILED		/* writing failed */
};

/* structure for funcs that get called by msg parse routine */
struct ups_handler
{
	int	(*setvar)(const char *, const char *);
	int	(*instcmd)(const char *, const char *);
};

