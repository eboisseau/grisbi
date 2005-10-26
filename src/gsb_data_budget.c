/* ************************************************************************** */
/* work with the struct of budget                                             */
/*                                                                            */
/*                                                                            */
/*     Copyright (C)	2000-2005 C�dric Auger (cedric@grisbi.org)	      */
/* 			http://www.grisbi.org				      */
/*                                                                            */
/*  This program is free software; you can redistribute it and/or modify      */
/*  it under the terms of the GNU General Public License as published by      */
/*  the Free Software Foundation; either version 2 of the License, or         */
/*  (at your option) any later version.                                       */
/*                                                                            */
/*  This program is distributed in the hope that it will be useful,           */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/*  GNU General Public License for more details.                              */
/*                                                                            */
/*  You should have received a copy of the GNU General Public License         */
/*  along with this program; if not, write to the Free Software               */
/*  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/*                                                                            */
/* ************************************************************************** */

/**
 * \file gsb_budget_data.c
 * work with the budget structure, no GUI here
 */


#include "include.h"

/*START_INCLUDE*/
#include "gsb_data_budget.h"
#include "gsb_data_category.h"
#include "gsb_data_transaction.h"
#include "categories_onglet.h"
#include "traitement_variables.h"
#include "dialog.h"
#include "include.h"
/*END_INCLUDE*/


/**
 * \struct 
 * Describe a budget
 */
typedef struct
{
    /** @name budget content */
    guint budget_number;
    gchar *budget_name;
    gint budget_type;		/**< 0:credit / 1:debit  */

    GSList *sub_budget_list;

    /** @name gui budget list content (not saved) */
    gint budget_nb_transactions;
    gint budget_nb_direct_transactions;
    gdouble budget_balance;
    gdouble budget_direct_balance;
} struct_budget;


/**
 * \struct 
 * Describe a sub-budget
 */
typedef struct
{
    /** @name sub-budget content */
    guint sub_budget_number;
    gchar *sub_budget_name;

    guint mother_budget_number;

    /** @name gui sub-budget list content (not saved)*/
    gint sub_budget_nb_transactions;
    gdouble sub_budget_balance;
} struct_sub_budget;

/**
 * \struct 
 * Describe a budget
 */
/* typedef struct */
/* { */
/*  */
/* } struct_transaction; */


/*START_STATIC*/
static GSList *gsb_data_budget_append_sub_budget_to_list ( GSList *budget_list,
						    GSList *sub_budget_list );
static void gsb_data_budget_create_default_budget_list ( void );
static gint gsb_data_budget_get_pointer_from_name_in_glist ( struct_budget *budget,
						      gchar *name );
static gint gsb_data_budget_get_pointer_from_sub_name_in_glist ( struct_sub_budget *sub_budget,
							  gchar *name );
static gpointer gsb_data_budget_get_structure_in_list ( gint no_budget,
						 GSList *list );
static gint gsb_data_budget_max_number ( void );
static gint gsb_data_budget_max_sub_budget_number ( gint budget_number );
static gboolean gsb_data_budget_merge_category_list ( void );
static gint gsb_data_budget_new ( gchar *name );
static gint gsb_data_budget_new_sub_budget ( gint budget_number,
				      gchar *name );
static void gsb_data_budget_reset_counters ( void );
/*END_STATIC*/

/*START_EXTERN*/
extern     gchar * buffer ;
/*END_EXTERN*/

/** contains the g_slist of struct_budget */
static GSList *budget_list;

/** a pointer to the last budget used (to increase the speed) */
static struct_budget *budget_buffer;
static struct_sub_budget *sub_budget_buffer;

/** a empty budget for the list of budgets */
static struct_budget *empty_budget;




/**
 * set the budgets global variables to NULL, usually when we init all the global variables
 *
 * \param none
 *
 * \return FALSE
 * */
gboolean gsb_data_budget_init_variables ( void )
{
    if ( budget_list )
	g_slist_free (budget_list);

    if ( empty_budget )
	free (empty_budget);

    budget_list = NULL;
    budget_buffer = NULL;
    sub_budget_buffer = NULL;

    /* create the empty budget */

    empty_budget = calloc ( 1,
			    sizeof ( struct_budget ));
    empty_budget -> budget_name = _("No budget line");

    return FALSE;
}



/**
 * find and return the structure of the budget asked
 *
 * \param no_budget number of budget
 *
 * \return the adr of the struct of the budget (NULL if doesn't exit)
 * */
gpointer gsb_data_budget_get_structure ( gint no_budget )
{
    if (!no_budget)
	return empty_budget;

    /* before checking all the budgets, we check the budget_buffer */

    if ( budget_buffer
	 &&
	 budget_buffer -> budget_number == no_budget )
	return budget_buffer;

    return gsb_data_budget_get_structure_in_list ( no_budget,
						   budget_list );
}



/**
 * find and return the structure of the budget in the list given in param
 * don't use at this level the buffer because could be a bug for an imported list
 * so for normal list, use always gsb_data_budget_get_structure
 *
 * \param no_budget number of budget
 * \param list the list of budgets struct where we look for
 *
 * \return the adr of the struct of the budget (NULL if doesn't exit)
 * */
gpointer gsb_data_budget_get_structure_in_list ( gint no_budget,
						 GSList *list )
{
    GSList *tmp;

    if (!no_budget)
	return NULL;

    tmp = list;

    while ( tmp )
    {
	struct_budget *budget;

	budget = tmp -> data;

	if ( budget -> budget_number == no_budget )
	{
	    budget_buffer = budget;
	    return budget;
	}
	tmp = tmp -> next;
    }
    return NULL;
}



/**
 * find and return the structure of the sub-budget asked
 *
 * \param no_budget number of budget
 * \param no_sub_budget the number of the sub-budget
 *
 * \return the adr of the struct of the sub-budget (NULL if doesn't exit)
 * */
gpointer gsb_data_budget_get_sub_budget_structure ( gint no_budget,
						    gint no_sub_budget )
{
    GSList *tmp;
    struct_budget *budget;

    if (!no_budget
	||
	!no_sub_budget)
	return NULL;

    /* before checking all the budgets, we check the buffer */

    if ( sub_budget_buffer
	 &&
	 sub_budget_buffer -> sub_budget_number == no_sub_budget )
	return sub_budget_buffer;

    budget = gsb_data_budget_get_structure ( no_budget );

    tmp = budget -> sub_budget_list;

    while ( tmp )
    {
	struct_sub_budget *sub_budget;

	sub_budget = tmp -> data;

	if ( sub_budget -> sub_budget_number == no_sub_budget )
	{
	    sub_budget_buffer = sub_budget;
	    return sub_budget;
	}
	tmp = tmp -> next;
    }
    return NULL;
}


/**
 * give the g_slist of budgets structure
 * usefull when want to check all budgets
 *
 * \param none
 *
 * \return the g_slist of budgets structure
 * */
GSList *gsb_data_budget_get_budgets_list ( void )
{
    return budget_list;
}


/**
 * return the g_slist of the sub-budgets of the budget
 *
 * \param no_budget the number of the budget
 *
 * \return a g_slist of the struct of the sub-budgets or NULL if problem
 * */
GSList *gsb_data_budget_get_sub_budget_list ( gint no_budget )
{
    struct_budget *budget;

    budget = gsb_data_budget_get_structure ( no_budget );

    if (!budget)
	return 0;

    return budget -> sub_budget_list;
}



/**
 * return the number of the budgets given in param
 *
 * \param budget_ptr a pointer to the struct of the budget
 *
 * \return the number of the budget, 0 if problem
 * */
gint gsb_data_budget_get_no_budget ( gpointer budget_ptr )
{
    struct_budget *budget;

    if ( !budget_ptr )
	return 0;

    budget = budget_ptr;
    budget_buffer = budget;
    return budget -> budget_number;
}


/**
 * return the number of the sub-budget given in param
 *
 * \param sub_budget_ptr a pointer to the struct of the sub-budget
 *
 * \return the number of the budget, 0 if problem
 * */
gint gsb_data_budget_get_no_sub_budget ( gpointer sub_budget_ptr )
{
    struct_sub_budget *sub_budget;

    if ( !sub_budget_ptr )
	return 0;

    sub_budget = sub_budget_ptr;
    sub_budget_buffer = sub_budget;
    return sub_budget -> sub_budget_number;
}


/**
 * return the number of the budget of the sub-budget given in param
 *
 * \param sub_budget_ptr a pointer to the struct of the sub-budget
 *
 * \return the number of the budget, 0 if problem
 * */
gint gsb_data_budget_get_no_budget_from_sub_budget ( gpointer sub_budget_ptr )
{
    struct_sub_budget *sub_budget;

    if ( !sub_budget_ptr )
	return 0;

    sub_budget = sub_budget_ptr;
    sub_budget_buffer = sub_budget;
    return sub_budget -> mother_budget_number;
}



/** find and return the last number of budget
 * \param none
 * \return last number of budget
 * */
gint gsb_data_budget_max_number ( void )
{
    GSList *tmp;
    gint number_tmp = 0;

    tmp = budget_list;

    while ( tmp )
    {
	struct_budget *budget;

	budget = tmp -> data;

	if ( budget -> budget_number > number_tmp )
	    number_tmp = budget -> budget_number;

	tmp = tmp -> next;
    }
    return number_tmp;
}


/**
 * find and return the last number of the sub-budgets
 *
 * \param 
 * 
 * \return last number of the sub-budgets
 * */
gint gsb_data_budget_max_sub_budget_number ( gint budget_number )
{
    struct_budget *budget;
    GSList *tmp;
    gint number_tmp = 0;

    budget = gsb_data_budget_get_structure ( budget_number );

    tmp = budget -> sub_budget_list;

    while ( tmp )
    {
	struct_sub_budget *sub_budget;

	sub_budget = tmp -> data;

	if ( sub_budget -> sub_budget_number > number_tmp )
	    number_tmp = sub_budget -> sub_budget_number;

	tmp = tmp -> next;
    }
    return number_tmp;
}



/**
 * create a new budget, give it a number, append it to the list
 * and return the number
 * update combofix and mark file as modified
 *
 * \param name the name of the budget (can be freed after, it's a copy) or NULL
 *
 * \return the number of the new budget
 * */
gint gsb_data_budget_new ( gchar *name )
{
    gint budget_number;

    /* create the new budget with a new number */

    budget_number = gsb_data_budget_new_with_number ( gsb_data_budget_max_number () + 1 );

    /* append the name if necessary */

    if (name)
	gsb_data_budget_set_name ( budget_number,
				   name );

    mise_a_jour_combofix_categ ();
    modification_fichier(TRUE);

    return budget_number;
}


/**
 * create a new budget with a number, append it to the list
 * and return the number
 * 
 *
 * \param number the number we want to give to that budget
 * \param import_list a g_slist with the imported budgets if we are importing them,
 * NULL else and the budget will be hapened to the normal budgets list
 *
 * \return the number of the new budget
 * */
gint gsb_data_budget_new_with_number ( gint number )
{
    struct_budget *budget;

    budget = calloc ( 1,
		      sizeof ( struct_budget ));
    budget -> budget_number = number;

    budget_list = g_slist_append ( budget_list,
				   budget );

    budget_buffer = budget;

    return budget -> budget_number;
}



/**
 * remove a budget
 * set all the budgets of transaction which are this one to 0
 * update combofix and mark file as modified
 *
 * \param no_budget the budget we want to remove
 *
 * \return TRUE ok
 * */
gboolean gsb_data_budget_remove ( gint no_budget )
{
    struct_budget *budget;

    budget = gsb_data_budget_get_structure ( no_budget );

    if (!budget)
	return FALSE;

    budget_list = g_slist_remove ( budget_list,
				   budget );

    mise_a_jour_combofix_categ ();
    modification_fichier(TRUE);
    return TRUE;
}


/**
 * remove a sub-budget from a budget
 * set all the budgets of transaction which are this one to 0
 * update combofix and mark file as modified
 *
 * \param no_budget the budget we want to remove
 *
 * \return TRUE ok
 * */
gboolean gsb_data_budget_sub_budget_remove ( gint no_budget,
					     gint no_sub_budget )
{
    struct_budget *budget;
    struct_sub_budget *sub_budget;

    budget = gsb_data_budget_get_structure ( no_budget );
    sub_budget = gsb_data_budget_get_sub_budget_structure ( no_budget,
							    no_sub_budget );

    if (!budget
	||
	!sub_budget)
	return FALSE;

    budget -> sub_budget_list = g_slist_remove ( budget -> sub_budget_list,
						 sub_budget );

    mise_a_jour_combofix_categ ();
    modification_fichier(TRUE);
    return TRUE;
}


/**
 * create a new sub-budget, append it to the list
 * and return the new number
 *
 * \param budget_number the number of the mother
 * \param name the name of the sub-budget
 *
 * \return the number of the new sub-budget or 0 if problem
 * */
gint gsb_data_budget_new_sub_budget ( gint budget_number,
				      gchar *name )
{
    gint sub_budget_number;

    sub_budget_number = gsb_data_budget_new_sub_budget_with_number ( gsb_data_budget_max_sub_budget_number (budget_number) + 1,
								     budget_number );

    /* append the name if necessary */

    if (name)
	gsb_data_budget_set_sub_budget_name ( budget_number,
					      sub_budget_number,
					      name );

    mise_a_jour_combofix_categ ();
    modification_fichier(TRUE);

    return budget_number;
}


/**
 * create a new sub-budget with a number, append it to the list
 * and return the number
 *
 * \param number the number we want to give to that sub-budget
 * \param budget_number the number of the mother
 *
 * \return the number of the new sub-budget or 0 if problem
 * */
gint gsb_data_budget_new_sub_budget_with_number ( gint number,
						  gint budget_number)
{
    struct_budget *budget;
    struct_sub_budget *sub_budget;

    budget = gsb_data_budget_get_structure ( budget_number );

    if (!budget)
	return 0;

    sub_budget = calloc ( 1,
			  sizeof (struct_sub_budget));
    sub_budget -> sub_budget_number = number;
    sub_budget -> mother_budget_number = budget_number;

    budget -> sub_budget_list = g_slist_append ( budget -> sub_budget_list,
						 sub_budget );

    return sub_budget -> sub_budget_number;
}



/**
 * return the number of the budget wich has the name in param
 * create it if necessary
 *
 * \param name the name of the budget
 * \param create TRUE if we want to create it if it doen't exist
 * \param budget_type the type of the budget if we create it
 *
 * \return the number of the budget or 0 if problem
 * */
gint gsb_data_budget_get_number_by_name ( gchar *name,
					  gboolean create,
					  gint budget_type )
{
    GSList *list_tmp;
    gint budget_number = 0;

    list_tmp = g_slist_find_custom ( budget_list,
				     name,
				     (GCompareFunc) gsb_data_budget_get_pointer_from_name_in_glist );

    if ( list_tmp )
    {
	struct_budget *budget;

	budget = list_tmp -> data;
	budget_number = budget -> budget_number;
    }
    else
    {
	if (create)
	{
	    budget_number = gsb_data_budget_new (name);
	    gsb_data_budget_set_type ( budget_number,
				       budget_type );
	}
    }
    return budget_number;
}



/**
 * return the number of the sub-budget wich has the name in param
 * create it if necessary
 *
 * \param budget_number the number of the budget
 * \param name the name of the sub-budget
 * \param create TRUE if we want to create it if it doen't exist
 *
 * \return the number of the sub-budget or 0 if problem
 * */
gint gsb_data_budget_get_sub_budget_number_by_name ( gint budget_number,
						     gchar *name,
						     gboolean create )
{
    GSList *list_tmp;
    struct_budget *budget;
    gint sub_budget_number = 0;

    budget = gsb_data_budget_get_structure ( budget_number );

    if (!budget)
	return 0;

    list_tmp = g_slist_find_custom ( budget -> sub_budget_list,
				     name,
				     (GCompareFunc) gsb_data_budget_get_pointer_from_sub_name_in_glist );

    if ( list_tmp )
    {
	struct_sub_budget *sub_budget;

	sub_budget = list_tmp -> data;
	sub_budget_number = sub_budget -> sub_budget_number;
    }
    else
    {
	if (create)
	    sub_budget_number = gsb_data_budget_new_sub_budget ( budget_number,
								 name);
    }
    return sub_budget_number;
}




/**
 * used with g_slist_find_custom to find a budget in the g_list
 * by his name
 *
 * \param budget the struct of the current budget checked
 * \param name the name we are looking for
 *
 * \return 0 if it's the same name
 * */
gint gsb_data_budget_get_pointer_from_name_in_glist ( struct_budget *budget,
						      gchar *name )
{
    return ( g_strcasecmp ( budget -> budget_name,
			    name ));
}


/**
 * used with g_slist_find_custom to find a sub-budget in the g_list
 * by his name
 *
 * \param sub_budget the struct of the current sub_budget checked
 * \param name the name we are looking for
 *
 * \return 0 if it's the same name
 * */
gint gsb_data_budget_get_pointer_from_sub_name_in_glist ( struct_sub_budget *sub_budget,
							  gchar *name )
{
    return ( g_strcasecmp ( sub_budget -> sub_budget_name,
			    name ));
}


/**
 * return the name of the budget
 * and the full name (ie budget : sub-budget if no_sub_budget is given)
 *
 * \param no_budget the number of the budget
 * \param no_sub_budget if we want the full name of the budget
 * \param return_value_error if problem, the value we return
 *
 * \return the name of the budget, budget : sub-budget or NULL/No budget if problem
 * */
gchar *gsb_data_budget_get_name ( gint no_budget,
				  gint no_sub_budget,
				  gchar *return_value_error )
{
    struct_budget *budget;
    gchar *return_value;

    budget = gsb_data_budget_get_structure ( no_budget );

    if (!budget)
	return return_value_error;

    return_value = budget -> budget_name;

    if ( no_sub_budget )
    {
	struct_sub_budget *sub_budget;

	sub_budget = gsb_data_budget_get_sub_budget_structure ( no_budget,
								no_sub_budget );

	if (sub_budget)
	    return_value = g_strconcat ( return_value,
					 " : ",
					 sub_budget -> sub_budget_name,
					 NULL );
    }
    return return_value;
}


/**
 * set the name of the budget
 * the value is dupplicate in memory
 *
 * \param no_budget the number of the budget
 * \param name the name of the budget
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_budget_set_name ( gint no_budget,
				    const gchar *name )
{
    struct_budget *budget;

    budget = gsb_data_budget_get_structure ( no_budget );

    if (!budget)
	return FALSE;

    /* we free the last name */

    if ( budget -> budget_name )
	free (budget -> budget_name);

    /* and copy the new one */
    if ( name )
	budget -> budget_name = g_strdup (name);
    else
	budget -> budget_name = NULL;
    return TRUE;
}

/**
 * return the name of the sub-budget
 *
 * \param no_budget the number of the budget
 * \param no_sub_budget the number of the sub-budget
 * \param return_value_error if problem, return that value
 *
 * \return the name of the budget or NULL/No sub-budget if problem
 * */
gchar *gsb_data_budget_get_sub_budget_name ( gint no_budget,
					     gint no_sub_budget,
					     gchar *return_value_error )
{
    struct_sub_budget *sub_budget;

    sub_budget = gsb_data_budget_get_sub_budget_structure ( no_budget,
							    no_sub_budget );

    if (!sub_budget)
	return (return_value_error);

    return sub_budget -> sub_budget_name;
}


/**
 * set the name of the sub-budget
 * the value is dupplicate in memory
 *
 * \param no_budget the number of the budget
 * \param no_sub_budget the number of the sub-budget
 * \param name the name of the sub-budget
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_budget_set_sub_budget_name ( gint no_budget,
					       gint no_sub_budget,
					       const gchar *name )
{
    struct_sub_budget *sub_budget;

    sub_budget = gsb_data_budget_get_sub_budget_structure ( no_budget,
							    no_sub_budget );

    if (!sub_budget)
	return FALSE;

    /* we free the last name */

    if ( sub_budget -> sub_budget_name )
	free (sub_budget -> sub_budget_name);

    /* and copy the new one */
    if ( name )
	sub_budget -> sub_budget_name = g_strdup (name);
    else
	sub_budget -> sub_budget_name = NULL;
    return TRUE;
}


/**
 * return a g_slist of g_slist of names of the budgets
 *
 * \param set_debit TRUE if we want to have the debits
 * \param set_credit TRUE if we want to have the credits
 *
 * \return a g_slist of g_slist of gchar *
 * */
GSList *gsb_data_budget_get_name_list ( gboolean set_debit,
					gboolean set_credit )
{
    GSList *return_list;
    GSList *tmp_list;
    GSList *debit_list = NULL;
    GSList *credit_list = NULL;

    return_list = NULL;

    /* fill debit_list and/or credit_list and them sub-budgets */

    tmp_list = budget_list;

    while ( tmp_list )
    {
	struct_budget *budget;

	budget = tmp_list -> data;

	if ( budget -> budget_type )
	{
	    if ( set_debit )
	    {
		debit_list = g_slist_append ( debit_list,
					      budget -> budget_name);
		debit_list = gsb_data_budget_append_sub_budget_to_list ( debit_list,
									 budget -> sub_budget_list);
	    }
	}
	else
	{
	    if ( set_credit )
	    {
		credit_list = g_slist_append ( credit_list,
					       budget -> budget_name);
		credit_list = gsb_data_budget_append_sub_budget_to_list ( credit_list,
									  budget -> sub_budget_list);
	    }
	}
	tmp_list = tmp_list -> next;
    }

    /* append what we need to return_list */

    if ( set_debit )
	return_list = g_slist_append ( return_list,
				       debit_list );
    if ( set_credit )
	return_list = g_slist_append ( return_list,
				       credit_list );

    return return_list;
}



/**
 * append the sub-budgets name with a tab at the begining
 * to the list of budgets given in param
 *
 * \param budget_list a g_slist of budgets names
 * \param sub_budget_list a g_slist which contains the sub budgets to append
 *
 * \return the new budget_list (normally shouldn't changed
 * */
GSList *gsb_data_budget_append_sub_budget_to_list ( GSList *budget_list,
						    GSList *sub_budget_list )
{
    GSList *tmp_list;

    if (!sub_budget_list)
	return budget_list;

    tmp_list = sub_budget_list;

    while (tmp_list)
    {
	struct_sub_budget *sub_budget;

	sub_budget = tmp_list -> data;

	budget_list = g_slist_append ( budget_list,
				       g_strconcat ( "\t",
						     sub_budget -> sub_budget_name,
						     NULL ));
	tmp_list = tmp_list -> next;
    }
    return budget_list;
}




/**
 * return the type of the budget
 * 0:credit / 1:debit / 2:special (transfert, breakdown...)
 *
 * \param no_budget the number of the budget
 * \param can_return_null if problem, return NULL if TRUE or "No budget" if FALSE
 *
 * \return the name of the budget or NULL/No budget if problem
 * */
gint gsb_data_budget_get_type ( gint no_budget )
{
    struct_budget *budget;

    budget = gsb_data_budget_get_structure ( no_budget );

    if (!budget)
	return 0;

    return budget -> budget_type;
}


/**
 * set the type of the budget
 * 0:credit / 1:debit / 2:special (transfert, breakdown...)
 *
 * \param no_budget the number of the budget
 * \param name the name of the budget
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_budget_set_type ( gint no_budget,
				    gint budget_type )
{
    struct_budget *budget;

    budget = gsb_data_budget_get_structure ( no_budget );

    if (!budget)
	return FALSE;

    budget -> budget_type = budget_type;
    return TRUE;
}


/**
 * return nb_transactions of the budget
 *
 * \param no_budget the number of the budget
 *
 * \return nb_transactions of the budget or 0 if problem
 * */
gint gsb_data_budget_get_nb_transactions ( gint no_budget )
{
    struct_budget *budget;

    budget = gsb_data_budget_get_structure ( no_budget );

    if (!budget)
	return 0;

    return budget -> budget_nb_transactions;
}



/**
 * return nb_transactions of the sub-budget
 *
 * \param no_budget the number of the budget
 * \param no_sub_budget the number of the sub-budget
 *
 * \return nb_transactions of the sub-budget or 0 if problem
 * */
gint gsb_data_budget_get_sub_budget_nb_transactions ( gint no_budget,
						      gint no_sub_budget )
{
    struct_sub_budget *sub_budget;

    sub_budget = gsb_data_budget_get_sub_budget_structure ( no_budget,
							    no_sub_budget );

    if (!sub_budget)
	return 0;

    return sub_budget -> sub_budget_nb_transactions;
}



/**
 * return nb_direct_transactions of the budget
 *
 * \param no_budget the number of the budget
 *
 * \return nb_direct_transactions of the budget or 0 if problem
 * */
gint gsb_data_budget_get_nb_direct_transactions ( gint no_budget )
{
    struct_budget *budget;

    budget = gsb_data_budget_get_structure ( no_budget );

    if (!budget)
	return 0;

    return budget -> budget_nb_direct_transactions;
}




/**
 * return balance of the budget
 *
 * \param no_budget the number of the budget
 *
 * \return balance of the budget or 0 if problem
 * */
gdouble gsb_data_budget_get_balance ( gint no_budget )
{
    struct_budget *budget;

    budget = gsb_data_budget_get_structure ( no_budget );

    if (!budget)
	return 0;

    return budget -> budget_balance;
}


/**
 * return balance of the sub-budget
 *
 * \param no_budget the number of the budget
 * \param no_sub_budget the number of the sub-budget
 *
 * \return balance of the sub-budget or 0 if problem
 * */
gdouble gsb_data_budget_get_sub_budget_balance ( gint no_budget,
						 gint no_sub_budget )
{
    struct_sub_budget *sub_budget;

    sub_budget = gsb_data_budget_get_sub_budget_structure ( no_budget,
							    no_sub_budget );

    if (!sub_budget)
	return 0;

    return sub_budget -> sub_budget_balance;
}

/**
 * return direct_balance of the budget
 *
 * \param no_budget the number of the budget
 *
 * \return balance of the budget or 0 if problem
 * */
gdouble gsb_data_budget_get_direct_balance ( gint no_budget )
{
    struct_budget *budget;

    budget = gsb_data_budget_get_structure ( no_budget );

    if (!budget)
	return 0;

    return budget -> budget_direct_balance;
}



/**
 * reset the counters of the budgets and sub-budgets
 *
 * \param 
 *
 * \return 
 * */
void gsb_data_budget_reset_counters ( void )
{
    GSList *list_tmp;

    list_tmp = budget_list;

    while ( list_tmp )
    {
	struct_budget *budget;
	GSList *sub_list_tmp;

	budget = list_tmp -> data;
	budget -> budget_balance = 0.0;
	budget -> budget_nb_transactions = 0;

	sub_list_tmp = budget -> sub_budget_list;

	while ( sub_list_tmp )
	{
	    struct_sub_budget *sub_budget;

	    sub_budget = sub_list_tmp -> data;

	    sub_budget -> sub_budget_nb_transactions = 0;
	    sub_budget -> sub_budget_balance = 0.0;

	    sub_list_tmp = sub_list_tmp -> next;
	}
	list_tmp = list_tmp -> next;
    }

    /* reset the empty budget */
    empty_budget -> budget_balance = 0.0;
    empty_budget -> budget_nb_transactions = 0;
}

/**
 * update the counters of the budgets
 *
 * \param
 *
 * \return
 * */
void gsb_data_budget_update_counters ( void )
{
    GSList *list_tmp_transactions;

    gsb_data_budget_reset_counters ();

    list_tmp_transactions = gsb_data_transaction_get_transactions_list ();

    while ( list_tmp_transactions )
    {
	gint transaction_number_tmp;
	transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);

	gsb_data_budget_add_transaction_to_budget ( transaction_number_tmp );

	list_tmp_transactions = list_tmp_transactions -> next;
    }
}


/**
 * add the given transaction to its budget in the counters
 * if the transaction has no budget, add it to the blank budget
 *
 * \param transaction_number the transaction we want to work with
 *
 * \return
 * */
void gsb_data_budget_add_transaction_to_budget ( gint transaction_number )
{
    struct_budget *budget;
    struct_sub_budget *sub_budget;

    budget = gsb_data_budget_get_structure ( gsb_data_transaction_get_budgetary_number (transaction_number));
    sub_budget = gsb_data_budget_get_sub_budget_structure ( gsb_data_transaction_get_budgetary_number (transaction_number),
							    gsb_data_transaction_get_sub_budgetary_number (transaction_number));

    if ( budget )
    {
	budget -> budget_nb_transactions ++;
	budget -> budget_balance += gsb_data_transaction_get_adjusted_amount (transaction_number);
    }
    else
    {
	empty_budget -> budget_nb_transactions ++;
	empty_budget -> budget_balance += gsb_data_transaction_get_adjusted_amount (transaction_number);
    }

    if ( sub_budget )
    {
	sub_budget -> sub_budget_nb_transactions ++;
	sub_budget -> sub_budget_balance += gsb_data_transaction_get_adjusted_amount (transaction_number);
    }
    else
    {
	if ( budget )
	{
	    budget -> budget_nb_direct_transactions ++;
	    budget -> budget_direct_balance += gsb_data_transaction_get_adjusted_amount (transaction_number);
	}
    }
}


/**
 * remove the given transaction to its budget in the counters
 * if the transaction has no budget, remove it to the blank budget
 *
 * \param transaction_number the transaction we want to work with
 *
 * \return
 * */
void gsb_data_budget_remove_transaction_from_budget ( gint transaction_number )
{
    struct_budget *budget;
    struct_sub_budget *sub_budget;

    budget = gsb_data_budget_get_structure ( gsb_data_transaction_get_budgetary_number (transaction_number));
    sub_budget = gsb_data_budget_get_sub_budget_structure ( gsb_data_transaction_get_budgetary_number (transaction_number),
							    gsb_data_transaction_get_sub_budgetary_number (transaction_number));

    if ( budget )
    {
	budget -> budget_nb_transactions --;
	budget -> budget_balance -= gsb_data_transaction_get_adjusted_amount (transaction_number);
	if ( !budget -> budget_nb_transactions ) /* Cope with float errors */
	    budget -> budget_balance = 0.0;
    }

    if ( sub_budget )
    {
	sub_budget -> sub_budget_nb_transactions --;
	sub_budget -> sub_budget_balance -= gsb_data_transaction_get_adjusted_amount (transaction_number);
	if ( !sub_budget -> sub_budget_nb_transactions ) /* Cope with float errors */
	    sub_budget -> sub_budget_balance = 0.0;
    }
    else
    {
	if ( budget )
	{
	    budget -> budget_nb_direct_transactions --;
	    budget -> budget_direct_balance -= gsb_data_transaction_get_adjusted_amount (transaction_number);
	}
    }
}



/**
 * create the default list of budgets
 * fill budget_list with the default budgets
 *
 * \param
 *
 * \return
 * */
void gsb_data_budget_create_default_budget_list ( void )
{
    /* FIXME : should ask here for different kind of budgets,
     * or leave them blank...
     * */

    /* for now, no default budget list */
}





/**
 * merge the given budget list with the current budget list
 *
 * \param list_to_merge
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_budget_merge_budget_list ( GSList *list_to_merge )
{
    GSList *list_tmp;

    list_tmp = list_to_merge;

    while ( list_tmp )
    {
	gint budget_number;
	struct_budget *new_budget;

	new_budget = list_tmp -> data;

	/* we try to find the new budget in the currents categories
	 * if don't, it creates it */

	budget_number = gsb_data_budget_get_number_by_name ( new_budget -> budget_name,
							     TRUE,
							     new_budget -> budget_type );

	/* we check budget_number but normally it will always != 0 */

	if ( budget_number )
	{
	    GSList *sub_list_tmp;

	    sub_list_tmp = new_budget -> sub_budget_list;

	    while ( sub_list_tmp )
	    {
		struct_sub_budget *new_sub_budget;

		new_sub_budget = sub_list_tmp -> data;

		gsb_data_budget_get_sub_budget_number_by_name ( budget_number,
								    new_sub_budget -> sub_budget_name,
								    TRUE );
		sub_list_tmp = sub_list_tmp -> next;
	    }
	    list_tmp = list_tmp -> next;
	}
    }
    return TRUE;
}



/**
 * merge the category list with the current budget list
 *
 * \param nothing
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_budget_merge_category_list ( void )
{
    if ( !question_yes_no_hint ( _("Merge the categories list"),
				 _("Warning: this will add all the categories and subcategories to the budgetary lines!\nBesides you can't cancel this afterwards.\nWe advise you not to use this unless you know exactly what you are doing.\nDo you want to continue anyway?")))
	return FALSE;
    
    return (gsb_data_budget_merge_budget_list (gsb_data_category_get_categories_list ()));
}




