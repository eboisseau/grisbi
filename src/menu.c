/* ************************************************************************** */
/* Ce fichier contient les définitions de tous les menus et barres d'outils   */
/*                                                                            */
/*                                  menu.c                                    */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2004-2009 Benjamin Drieu (bdrieu@april.org)                       */
/*          http://www.grisbi.org                                             */
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "menu.h"
#include "custom_list.h"
#include "export.h"
#include "fenetre_principale.h"
#include "file_obfuscate_qif.h"
#include "file_obfuscate.h"
#include "gsb_account.h"
#include "gsb_assistant_account.h"
#include "gsb_assistant_archive.h"
#include "gsb_assistant_archive_export.h"
#include "gsb_data_account.h"
#include "gsb_data_mix.h"
#include "gsb_debug.h"
#include "gsb_dirs.h"
#include "gsb_file.h"
#include "gsb_form.h"
#include "gsb_scheduler_list.h"
#include "gsb_transactions_list.h"
#include "help.h"
#include "import.h"
#include "main.h"
#include "navigation.h"
#include "parametres.h"
#include "structures.h"
#include "traitement_variables.h"
#include "tip.h"
#include "utils.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean gsb_gui_toggle_show_closed_accounts ( void );
static gboolean gsb_gui_toggle_show_form ( void );

static gboolean help_bugreport ( void );
static gboolean help_manual ( void );
static gboolean help_quick_start ( void );
static gboolean help_translation ( void );
static gboolean help_website ( void );
static gboolean gsb_menu_reinit_largeur_col_menu ( void );
/*END_STATIC*/



/*START_EXTERN*/
extern gchar **tab_noms_derniers_fichiers_ouverts;
/*END_EXTERN*/


static gboolean block_menu_cb = FALSE;
static GtkUIManager *ui_manager;
static gint merge_id = -1;
static gint recent_files_merge_id = -1;
static gint move_to_account_merge_id = -1;

static const gchar *ui_manager_buffer =
"<ui>"
"  <menubar>"
"    <menu name='FileMenu' action='FileMenuAction' >"
"      <menuitem name='New' action='NewAction'/>"
"      <menuitem name='Open' action='OpenAction'/>"
"      <menu name='RecentFiles' action='RecentFilesAction'>"
"      </menu>"
"      <separator/>"
"      <menuitem name='Save' action='SaveAction'/>"
"      <menuitem name='SaveAs' action='SaveAsAction'/>"
"      <separator/>"
"      <menuitem name='ImportFile' action='ImportFileAction'/>"
"      <menuitem name='ExportFile' action='ExportFileAction'/>"
"      <separator/>"
"      <menuitem name='CreateArchive' action='CreateArchiveAction'/>"
"      <menuitem name='ExportArchive' action='ExportArchiveAction'/>"
"      <separator/>"
"      <menuitem name='DebugFile' action='DebugFileAction'/>"
"      <menuitem name='Obfuscate' action='ObfuscateAction'/>"
"      <menuitem name='ObfuscateQif' action='ObfuscateQifAction'/>"
"      <menuitem name='DebugMode' action='DebugModeAction'/>"
"      <separator/>"
"      <menuitem name='Close' action='CloseAction'/>"
"      <menuitem name='Quit' action='QuitAction'/>"
"    </menu>"
"    <menu name='EditMenu' action='EditMenuAction' >"
"      <menuitem name='EditTransaction' action='EditTransactionAction'/>"
"      <separator/>"
"      <menuitem name='NewTransaction' action='NewTransactionAction'/>"
"      <menuitem name='RemoveTransaction' action='RemoveTransactionAction'/>"
"      <menuitem name='TemplateTransaction' action='TemplateTransactionAction'/>"
"      <menuitem name='CloneTransaction' action='CloneTransactionAction'/>"
"      <separator/>"
"      <menuitem name='ConvertToScheduled' action='ConvertToScheduledAction'/>"
"      <menu name='MoveToAnotherAccount' action='MoveToAnotherAccountAction'>"
"      </menu>"
"   <separator/>"
"       <menuitem name='NewAccount' action='NewAccountAction'/>"
"       <menuitem name='RemoveAccount' action='RemoveAccountAction'/>"
#ifndef GTKOSXAPPLICATION
"      <separator/>"
#endif
"      <menuitem name='Preferences' action='PrefsAction'/>"
"    </menu>"
"    <menu name='ViewMenu' action='ViewMenuAction'>"
"      <menuitem name='ShowTransactionForm' action='ShowTransactionFormAction'/>"
"      <menuitem name='ShowReconciled' action='ShowReconciledAction'/>"
"      <menuitem name='ShowArchived' action='ShowArchivedAction'/>"
"      <menuitem name='ShowClosed' action='ShowClosedAction'/>"
"      <separator/>"
"      <menuitem name='ShowOneLine' action='ShowOneLineAction'/>"
"      <menuitem name='ShowTwoLines' action='ShowTwoLinesAction'/>"
"      <menuitem name='ShowThreeLines' action='ShowThreeLinesAction'/>"
"      <menuitem name='ShowFourLines' action='ShowFourLinesAction'/>"
"      <separator/>"
"      <menuitem name='InitwidthCol' action='InitwidthColAction'/>"
"    </menu>"
"    <menu name='Help' action='HelpMenuAction' >"
"      <menuitem name='Manual' action='ManualAction'/>"
"      <menuitem name='QuickStart' action='QuickStartAction'/>"
/*"      <menuitem name='Translation' action='TranslationAction'/>"*/
"      <menuitem name='About' action='AboutAction'/>"
"      <separator/>"
"      <menuitem name='GrisbiWebsite' action='GrisbiWebsiteAction'/>"
"      <menuitem name='ReportBug' action='ReportBugAction'/>"
"      <separator/>"
"      <menuitem name='Tip' action='TipAction'/>"
"    </menu>"
"  </menubar>"
"</ui>";


GtkWidget *init_menus ( GtkWidget *vbox )
{
    GtkWidget *menubar;
    GtkActionGroup *actions;

    /* remind of GtkActionEntry : name, stock_id, label, accelerator, tooltip, callback */
    GtkActionEntry entries[] =
    {
        /* File menu */
        {"FileMenuAction", NULL, _("_File"), NULL, NULL, NULL},
	#ifdef GTKOSXAPPLICATION
        { "NewAction", GTK_STOCK_NEW, _("_New account file..."), "<Meta>N", NULL,
         G_CALLBACK ( gsb_file_new ) },
        {"OpenAction",  GTK_STOCK_OPEN, _("_Open..."), "<Meta>O", NULL,
         G_CALLBACK ( gsb_file_open_menu ) },
        {"RecentFilesAction", NULL, _("_Recently opened files"), NULL, NULL, NULL },
        {"SaveAction", GTK_STOCK_SAVE, _("_Save"), "<Meta>S", NULL,
         G_CALLBACK ( gsb_file_save ) },
	#else
        { "NewAction", GTK_STOCK_NEW, _("_New account file..."), NULL, NULL,
         G_CALLBACK ( gsb_file_new ) },
        {"OpenAction",  GTK_STOCK_OPEN, _("_Open..."), NULL, NULL,
         G_CALLBACK ( gsb_file_open_menu ) },
        {"RecentFilesAction", NULL, _("_Recently opened files"), NULL, NULL, NULL },
        {"SaveAction", GTK_STOCK_SAVE, _("_Save"), NULL, NULL,
         G_CALLBACK ( gsb_file_save ) },
	#endif
        {"SaveAsAction", GTK_STOCK_SAVE_AS,	_("_Save as..."), NULL, NULL,
         G_CALLBACK ( gsb_file_save_as ) },
        {"ImportFileAction", GTK_STOCK_CONVERT,	_("_Import file..."), NULL, NULL,
         G_CALLBACK ( importer_fichier ) },
        { "ExportFileAction", GTK_STOCK_CONVERT, _("_Export accounts as QIF/CSV file..."), NULL, NULL,
         G_CALLBACK ( export_accounts ) },
        {"CreateArchiveAction", GTK_STOCK_CLEAR, _("Archive transactions..."), NULL, NULL,
         G_CALLBACK ( gsb_assistant_archive_run_by_menu ) },
        {"ExportArchiveAction",	GTK_STOCK_HARDDISK,	_("_Export an archive as GSB/QIF/CSV file..."), NULL, NULL,
         G_CALLBACK ( gsb_assistant_archive_export_run ) },
        {"DebugFileAction", GTK_STOCK_FIND, _("_Debug account file..."), "", NULL,
         G_CALLBACK ( gsb_debug ) },
        {"ObfuscateAction", GTK_STOCK_FIND, _("_Obfuscate account file..."), "", NULL,
         G_CALLBACK ( file_obfuscate_run ) },
        {"ObfuscateQifAction", GTK_STOCK_FIND, _("_Obfuscate QIF file..."), "", NULL,
         G_CALLBACK ( file_obfuscate_qif_run ) },
	#ifdef GTKOSXAPPLICATION
        {"CloseAction", GTK_STOCK_CLOSE, _("_Close"), "<Meta>W", NULL,
         G_CALLBACK ( gsb_file_close ) },
	#else
        {"CloseAction", GTK_STOCK_CLOSE, _("_Close"), NULL, NULL,
         G_CALLBACK ( gsb_file_close ) },
	#endif
        {"QuitAction", GTK_STOCK_QUIT, _("_Quit"), NULL, NULL,
         G_CALLBACK ( gsb_main_grisbi_close ) },

        /* Editmenu */
        {"EditMenuAction", NULL, _("_Edit"), NULL, NULL, NULL },
        {"EditTransactionAction", GTK_STOCK_EDIT, _("_Edit transaction"), "", NULL,
         G_CALLBACK ( gsb_data_mix_edit_current_transaction ) },
        {"NewTransactionAction", GTK_STOCK_NEW, _("_New transaction"), "<Control>T", NULL,
         G_CALLBACK ( gsb_data_mix_new_transaction_by_menu ) },
        {"RemoveTransactionAction", GTK_STOCK_DELETE, _("_Remove transaction"), "", NULL,
         G_CALLBACK ( gsb_data_mix_delete_current_transaction ) },
        {"TemplateTransactionAction", GTK_STOCK_COPY, _("Use selected transaction as a template"), "", NULL,
         G_CALLBACK ( gsb_transactions_list_clone_template ) },
        {"CloneTransactionAction", GTK_STOCK_COPY, _("_Clone transaction"), "", NULL,
         G_CALLBACK ( gsb_data_mix_clone_current_transaction ) },
        {"ConvertToScheduledAction", GTK_STOCK_CONVERT, _("Convert to _scheduled transaction"), NULL, NULL,
         G_CALLBACK ( schedule_selected_transaction ) },
        {"MoveToAnotherAccountAction", NULL, _("_Move transaction to another account"), NULL, NULL, NULL },
        {"NewAccountAction", GTK_STOCK_NEW, _("_New account"), "", NULL,
         G_CALLBACK ( gsb_assistant_account_run ) },
        {"RemoveAccountAction", GTK_STOCK_DELETE, _("_Remove current account"), "", NULL,
         G_CALLBACK ( gsb_account_delete ) },
        {"PrefsAction", GTK_STOCK_PREFERENCES, _("_Preferences"), NULL, NULL,
         G_CALLBACK ( preferences ) },

        /* View menu */
        {"ViewMenuAction", NULL, _("_View"), NULL, NULL, NULL },
        {"InitwidthColAction", NULL, _("Reset the column width"), NULL, NULL,
         G_CALLBACK ( gsb_menu_reinit_largeur_col_menu ) },

        /* Help menu */
        {"HelpMenuAction", NULL, _("_Help"), NULL, NULL, NULL },
	#ifdef GTKOSXAPPLICATION
        {"ManualAction", GTK_STOCK_HELP, _("_Manual"), "<Meta>H", NULL,
         G_CALLBACK ( help_manual ) },
	#else
        {"ManualAction", GTK_STOCK_HELP, _("_Manual"), NULL, NULL,
         G_CALLBACK ( help_manual ) },
	#endif
        {"QuickStartAction", NULL, _("_Quick start"), NULL, NULL,
         G_CALLBACK ( help_quick_start ) },
        /* {"TranslationAction", NULL, _("_Translation"), NULL, NULL,
         G_CALLBACK ( help_translation ) }, */
        {"AboutAction", GTK_STOCK_ABOUT, _("_About Grisbi..."), NULL, NULL,
         G_CALLBACK ( a_propos ) },
        {"GrisbiWebsiteAction", NULL, _("_Grisbi website"), NULL, NULL,
         G_CALLBACK ( help_website ) },
        {"ReportBugAction", NULL, _("_Report a bug"), NULL, NULL,
         G_CALLBACK ( help_bugreport ) },
        {"TipAction", GTK_STOCK_DIALOG_INFO, _("_Tip of the day"), NULL, NULL,
         G_CALLBACK ( force_display_tip ) },
    };

    GtkRadioActionEntry radio_entries[] =
    {
        /* Name, StockID, Label, Accelerator, Tooltip, Value */
        {"ShowOneLineAction", NULL, _("Show _one line per transaction"), NULL, NULL,
         ONE_LINE_PER_TRANSACTION },
        {"ShowTwoLinesAction", NULL, _("Show _two lines per transaction"), NULL, NULL,
         TWO_LINES_PER_TRANSACTION },
        {"ShowThreeLinesAction", NULL, _("Show _three lines per transaction"), NULL, NULL,
         THREE_LINES_PER_TRANSACTION },
        {"ShowFourLinesAction", NULL, _("Show _four lines per transaction"), NULL, NULL,
         FOUR_LINES_PER_TRANSACTION },
    };

    GtkToggleActionEntry toggle_entries[] =
    {
        {"DebugModeAction", NULL, _("Debug mode"), NULL, NULL,
         G_CALLBACK ( gsb_debug_start_log ), etat.debug_mode },
        {"ShowTransactionFormAction", NULL, _("Show transaction _form"), NULL, NULL,
         G_CALLBACK ( gsb_gui_toggle_show_form ), conf.formulaire_toujours_affiche },
#ifdef GTKOSXAPPLICATION
        {"ShowReconciledAction", NULL, _("Show _reconciled"), "<Meta>R", NULL,
         G_CALLBACK ( gsb_gui_toggle_show_reconciled ), 0 },
        {"ShowArchivedAction", NULL, _("Show _lines archives"), "<Meta>L", NULL,
         G_CALLBACK ( gsb_gui_toggle_show_archived ), 0 },
#else
        {"ShowReconciledAction", NULL, _("Show _reconciled"), "<Alt>R", NULL,
         G_CALLBACK ( gsb_gui_toggle_show_reconciled ), 0 },
        {"ShowArchivedAction", NULL, _("Show _lines archives"), "<Alt>L", NULL,
         G_CALLBACK ( gsb_gui_toggle_show_archived ), 0 },
#endif
        {"ShowClosedAction", NULL, _("Show _closed accounts"), NULL, NULL,
         G_CALLBACK ( gsb_gui_toggle_show_closed_accounts ), conf.show_closed_accounts }
    };

    ui_manager = gtk_ui_manager_new ();

    actions = gtk_action_group_new ( "Actions" );

    gtk_action_group_add_actions (actions,
                        entries,
                        G_N_ELEMENTS ( entries ),
                        (gpointer) run.window );

    gtk_action_group_add_radio_actions ( actions,
                        radio_entries,
                        G_N_ELEMENTS ( radio_entries ),
                        -1,
                        G_CALLBACK ( gsb_gui_toggle_line_view_mode ),
                        NULL );

    gtk_action_group_add_toggle_actions ( actions,
                        toggle_entries,
                        G_N_ELEMENTS ( toggle_entries ),
                        NULL );

    gtk_ui_manager_insert_action_group ( ui_manager, actions, 0 );
    g_object_unref ( G_OBJECT ( actions ) );

    merge_id = gtk_ui_manager_add_ui_from_string ( ui_manager,
                        ui_manager_buffer, -1, NULL );

#ifndef GTKOSXAPPLICATION
    gtk_window_add_accel_group ( GTK_WINDOW ( run.window ),
                        gtk_ui_manager_get_accel_group ( ui_manager ) );
#endif /* GTKOSXAPPLICATION */

    menubar = gtk_ui_manager_get_widget ( ui_manager, "/menubar" );
    gtk_box_pack_start ( GTK_BOX ( vbox ),  menubar, FALSE, TRUE, 0 );

    /* return */
    return menubar;
}


/**
 * Blank the "Recent files submenu".
 */
void efface_derniers_fichiers_ouverts ( void )
{
    gtk_ui_manager_remove_ui ( ui_manager, recent_files_merge_id );
}


/**
 * Add menu items to the "Recent files" submenu.
 */
gboolean affiche_derniers_fichiers_ouverts ( void )
{
    gint i;
    GtkActionGroup * action_group;

    efface_derniers_fichiers_ouverts ();

    if ( conf.nb_derniers_fichiers_ouverts > conf.nb_max_derniers_fichiers_ouverts )
    {
        conf.nb_derniers_fichiers_ouverts = conf.nb_max_derniers_fichiers_ouverts;
    }

    if ( ! conf.nb_derniers_fichiers_ouverts || ! conf.nb_max_derniers_fichiers_ouverts )
    {
        return FALSE;
    }

    action_group = gtk_action_group_new ( "Group2" );

    for ( i = 0 ; i < conf.nb_derniers_fichiers_ouverts ; i++ )
    {
        gchar *tmp_name;
        GtkAction *action;

        tmp_name = g_strdup_printf ( "LastFile%d", i );

        action = gtk_action_new ( tmp_name,
                        tab_noms_derniers_fichiers_ouverts[i],
                        "",
                        "" );
        g_free ( tmp_name );
        g_signal_connect ( action,
                        "activate",
                        G_CALLBACK ( gsb_file_open_direct_menu ),
                        GINT_TO_POINTER ( i ) );
        gtk_action_group_add_action ( action_group, action );
    }

    gtk_ui_manager_insert_action_group ( ui_manager, action_group, 1 );
    g_object_unref ( G_OBJECT ( action_group ) );

    recent_files_merge_id = gtk_ui_manager_new_merge_id ( ui_manager );

    for ( i=0 ; i < conf.nb_derniers_fichiers_ouverts ; i++ )
    {
        gchar *tmp_name;
        gchar *tmp_label;

        tmp_name = g_strdup_printf ( "LastFile%d", i );
        tmp_label = g_strdup_printf ( "_%d LastFile%d", i, i );

        gtk_ui_manager_add_ui ( ui_manager,
                    recent_files_merge_id,
                    "/menubar/FileMenu/RecentFiles/",
                    tmp_label,
                    tmp_name,
                    GTK_UI_MANAGER_MENUITEM,
                    FALSE );

        g_free ( tmp_name );
        g_free ( tmp_label );
    }

    /* add a separator */
    gtk_ui_manager_add_ui ( ui_manager,
                    merge_id,
                    "/menubar/FileMenu/Open/",
                    NULL,
                    NULL,
                    GTK_UI_MANAGER_SEPARATOR,
                    FALSE );

    gtk_ui_manager_ensure_update ( ui_manager );

#ifdef GTKOSXAPPLICATION
    grisbi_osx_app_update_menus_cb ( );
#endif /* GTKOSXAPPLICATION */
    return FALSE;
}



/**
 * Start a browser processus with local copy of manual on command
 * line.
 *
 * \return FALSE
 */
gboolean help_manual ( void )
{
    gchar *lang = _("en");
    gchar *string;

    string = g_build_filename ( gsb_dirs_get_help_dir (), lang, "manual.html", NULL );

    if (g_file_test ( string,
		      G_FILE_TEST_EXISTS ))
    {
	lance_navigateur_web (string);
	g_free (string);
    }
    else
    {
	g_free (string);
	string = g_build_filename ( gsb_dirs_get_help_dir (), lang, "grisbi-manuel.html", NULL );
	lance_navigateur_web (string);
	g_free (string);
    }

    return FALSE;
}



/**
 * Start a browser processus with local copy of the quick start page
 * on command line.
 *
 * \return FALSE
 */
gboolean help_quick_start ( void )
{
    gchar *lang = _("en");

    gchar* tmpstr = g_build_filename ( HELP_PATH, lang, "quickstart.html", NULL );
    lance_navigateur_web ( tmpstr );
    g_free ( tmpstr );

    return FALSE;
}



/**
 * Start a browser processus with local copy of the translation page
 * on command line.
 *
 * \return FALSE
 */
gboolean help_translation ( void )
{
    gchar *lang = _("en");

    gchar* tmpstr = g_build_filename ( HELP_PATH, lang, "translation.html", NULL );
    lance_navigateur_web ( tmpstr );
    g_free ( tmpstr );

    return FALSE;
}



/**
 * Start a browser processus with Grisbi website displayed.
 *
 * \return FALSE
 */
gboolean help_website ( void )
{
    lance_navigateur_web ( "http://www.grisbi.org/" );

    return FALSE;
}



/**
 * Start a browser processus with Grisbi bug report page displayed.
 *
 * \return FALSE
 */
gboolean help_bugreport ( void )
{
    lance_navigateur_web ( "http://www.grisbi.org/bugsreports/" );

    return FALSE;
}



/**
 * Set sensitiveness of a menu item according to a string
 * representation of its position in the menu.
 * menu.
 *
 * \param item_name		Path of the menu item.
 * \param state			Whether widget should be 'sensitive' or not.
 *
 * \return TRUE on success.
 */
gboolean gsb_gui_sensitive_menu_item ( gchar *item_name, gboolean state )
{
    GtkWidget * widget;

    widget = gtk_ui_manager_get_widget ( ui_manager, item_name );

    if ( widget && GTK_IS_WIDGET(widget) )
    {
	gtk_widget_set_sensitive ( widget, state );
	return TRUE;
    }
    return FALSE;
}



/**
 * Callback called when an item of the "View/Show ... lines" menu is
 * triggered.
 */
void gsb_gui_toggle_line_view_mode ( GtkRadioAction *action,
                        GtkRadioAction *current,
                        gpointer user_data )
{
    /* FIXME benj: ugly but I cannot find a way to block this ... */
    if ( block_menu_cb ) return;

    switch ( gtk_radio_action_get_current_value(current) )
    {
	case ONE_LINE_PER_TRANSACTION:
	    change_aspect_liste (1);
	    break;
	case TWO_LINES_PER_TRANSACTION:
	    change_aspect_liste (2);
	    break;
	case THREE_LINES_PER_TRANSACTION:
	    change_aspect_liste (3);
	    break;
	case FOUR_LINES_PER_TRANSACTION:
	    change_aspect_liste (4);
	    break;
    }
}



/**
 * Show or hide the transactions form.
 *
 * \return FALSE
 */
gboolean gsb_gui_toggle_show_form ( void )
{
    devel_debug (NULL);

    /* FIXME benj: ugly but I cannot find a way to block this ... */
    if ( block_menu_cb )
        return FALSE;

    gsb_form_switch_expander ( );

    return FALSE;
}



/**
 * Show or hide display of reconciled transactions.
 *
 * \return FALSE
 */
gboolean gsb_gui_toggle_show_reconciled ( void )
{
    gint current_account;

    if ( block_menu_cb )
	    return FALSE;

    current_account = gsb_gui_navigation_get_current_account ( );
    if ( current_account == -1 || run.equilibrage == 1 )
        return FALSE;

    if ( gsb_data_account_get_r ( current_account ) )
	    change_aspect_liste ( 6 );
    else
	    change_aspect_liste ( 5 );

    return FALSE;
}


/**
 * Show or hide display of lines archives.
 *
 * \return FALSE
 */
gboolean gsb_gui_toggle_show_archived ( void )
{
    gint current_account;

    if ( block_menu_cb )
	    return FALSE;

    current_account = gsb_gui_navigation_get_current_account ( );
    if ( current_account == -1 )
        return FALSE;

    if ( gsb_data_account_get_l ( current_account ) )
	    change_aspect_liste ( 8 );
    else
	    change_aspect_liste ( 7 );

    return FALSE;
}


/**
 * Show or hide closed accounts.
 *
 * \return FALSE
 */
gboolean gsb_gui_toggle_show_closed_accounts ( void )
{
    conf.show_closed_accounts = !conf.show_closed_accounts;

    gsb_gui_navigation_create_account_list ( gsb_gui_navigation_get_model ( ) );
    gsb_gui_navigation_update_home_page ( );

    gsb_file_set_modified ( TRUE );

    return FALSE;
}



/**
 * Update the view menu in the menu bar
 *
 * \param account_number	The account used to update the menu
 *
 * \return FALSE
 * */
gboolean gsb_menu_update_view_menu ( gint account_number )
{
    gchar * item_name = NULL;
    gchar *tmpstr;

    devel_debug_int (account_number);

    block_menu_cb = TRUE;

    /* update the showing of reconciled transactions */
    tmpstr = "/menubar/ViewMenu/ShowReconciled";
    gtk_toggle_action_set_active ( GTK_TOGGLE_ACTION (
                        gtk_ui_manager_get_action ( ui_manager, tmpstr) ),
				        gsb_data_account_get_r ( account_number ) );

    tmpstr = "/menubar/ViewMenu/ShowTransactionForm";
    gtk_toggle_action_set_active ( GTK_TOGGLE_ACTION (
                        gtk_ui_manager_get_action ( ui_manager, tmpstr) ),
				        gsb_form_is_visible ( ) );

    /* update the showing of archived transactions */
    tmpstr = "/menubar/ViewMenu/ShowArchived";
    gtk_toggle_action_set_active ( GTK_TOGGLE_ACTION (
                        gtk_ui_manager_get_action ( ui_manager, tmpstr) ),
				        gsb_data_account_get_l ( account_number ) );

    /* update the number of line showed */
    switch ( gsb_data_account_get_nb_rows (account_number))
    {
	default:
	case 1 :
	    item_name = "/menubar/ViewMenu/ShowOneLine";
	    break;
	case 2 :
	    item_name = "/menubar/ViewMenu/ShowTwoLines";
	    break;
	case 3 :
	    item_name = "/menubar/ViewMenu/ShowThreeLines";
	    break;
	case 4 :
	    item_name = "/menubar/ViewMenu/ShowFourLines";
	    break;
    }

    gtk_toggle_action_set_active ( GTK_TOGGLE_ACTION (
                        gtk_ui_manager_get_action ( ui_manager, item_name ) ),
				        TRUE );
    block_menu_cb = FALSE;

    return FALSE;
}


/**
 * Update the clickable list of closed accounts and target
 * accounts to move a transaction, in menu.
 *
 * \param
 * \return FALSE
 * */
gboolean gsb_menu_update_accounts_in_menus ( void )
{
    GSList *list_tmp;
    GtkActionGroup * action_group;

    if ( move_to_account_merge_id != -1 )
        gtk_ui_manager_remove_ui ( ui_manager, move_to_account_merge_id );

    move_to_account_merge_id = gtk_ui_manager_new_merge_id ( ui_manager );
    action_group = gtk_action_group_new ( "Group3" );

    /* create the closed accounts and accounts in the menu to move a transaction */
    list_tmp = gsb_data_account_get_list_accounts ();

    while ( list_tmp )
    {
        gint i;

        i = gsb_data_account_get_no_account ( list_tmp -> data );

        if ( !gsb_data_account_get_closed_account ( i ) )
        {
            gchar *tmp_name;
            gchar *account_name;
            GtkAction *action;

            tmp_name = g_strdup_printf ( "MoveToAccount%d", i );
            account_name = gsb_data_account_get_name ( i );
            if ( !account_name )
                account_name = _("Unnamed account");

            action = gtk_action_new ( tmp_name, account_name, "", "" );

            if ( gsb_gui_navigation_get_current_account () == i )
                gtk_action_set_sensitive ( action, FALSE );

            gtk_action_group_add_action ( action_group, action );
            g_signal_connect ( action,
                        "activate",
                        G_CALLBACK ( move_selected_operation_to_account_nb ),
                        GINT_TO_POINTER ( i ) );

            gtk_ui_manager_add_ui ( ui_manager,
                        move_to_account_merge_id,
                        "/menubar/EditMenu/MoveToAnotherAccount/",
                        tmp_name,
                        tmp_name,
                        GTK_UI_MANAGER_MENUITEM,
                        FALSE );
            g_object_unref ( G_OBJECT ( action ) );
            g_free ( tmp_name );
        }

        list_tmp = list_tmp -> next;
    }

    gtk_ui_manager_insert_action_group ( ui_manager, action_group, 2 );
    gtk_ui_manager_ensure_update ( ui_manager );
    g_object_unref ( G_OBJECT ( action_group ) );

    return FALSE;
}



/**
 * Set sensitiveness of all menu items that work on the selected transaction.

 * \param sensitive	Sensitiveness (FALSE for unsensitive, TRUE for
 *			sensitive).
 *
 * \return		FALSE
 */
gboolean gsb_menu_set_menus_select_transaction_sensitive ( gboolean sensitive )
{
    devel_debug ( sensitive ? "item sensitive" : "item unsensitive" );

    gsb_gui_sensitive_menu_item ( "/menubar/EditMenu/EditTransaction", sensitive );
    gsb_gui_sensitive_menu_item ( "/menubar/EditMenu/RemoveTransaction", sensitive );
    gsb_gui_sensitive_menu_item ( "/menubar/EditMenu/TemplateTransaction", sensitive );
    gsb_gui_sensitive_menu_item ( "/menubar/EditMenu/CloneTransaction", sensitive );
    gsb_gui_sensitive_menu_item ( "/menubar/EditMenu/ConvertToScheduled", sensitive );
    gsb_gui_sensitive_menu_item ( "/menubar/EditMenu/MoveToAnotherAccount", sensitive );

    return FALSE;
}


/**
 * Set sensitiveness of all menu items that work on the selected scheduled.

 * \param sensitive	Sensitiveness (FALSE for unsensitive, TRUE for
 *			sensitive).
 *
 * \return		FALSE
 */
gboolean gsb_menu_set_menus_select_scheduled_sensitive ( gboolean sensitive )
{
    devel_debug ( sensitive ? "item sensitive" : "item unsensitive" );

    gsb_gui_sensitive_menu_item ( "/menubar/EditMenu/EditTransaction", sensitive );
    gsb_gui_sensitive_menu_item ( "/menubar/EditMenu/RemoveTransaction", sensitive );
    gsb_gui_sensitive_menu_item ( "/menubar/EditMenu/CloneTransaction", sensitive );

    return FALSE;
}


/**
 *
 *
 *
 *
 **/
GtkUIManager *gsb_menu_get_ui_manager ( void )
{
    return ui_manager;
}


/**
 *
 *
 *
 *
 **/
void gsb_menu_free_ui_manager ( void )
{
    if ( ! ui_manager )
        return;

    g_object_unref ( G_OBJECT ( ui_manager ) );
    ui_manager = NULL;
}


/**
 *
 *
 *
 *
 **/
gboolean gsb_menu_reinit_largeur_col_menu ( void )
{
    gint current_page;

    current_page = gsb_gui_navigation_get_current_page ( );

    if ( current_page == GSB_ACCOUNT_PAGE )
    {
        initialise_largeur_colonnes_tab_affichage_ope ( GSB_ACCOUNT_PAGE, NULL );

        gsb_transactions_list_set_largeur_col ( );
    }
    else if ( current_page == GSB_SCHEDULER_PAGE )
    {
        initialise_largeur_colonnes_tab_affichage_ope ( GSB_SCHEDULER_PAGE, NULL );

        gsb_scheduler_list_set_largeur_col ( );
    }

    return FALSE;
}


/**
 *
 *
 *
 */
gboolean gsb_menu_set_block_menu_cb ( gboolean etat )
{

    block_menu_cb = etat;

    return FALSE;
}


/**
 * Initialise la barre de menus en fonction de la présence ou non d'un fichier de comptes
 *
 * \param sensitif
 *
 * \return
 * */
void gsb_menu_set_menus_with_file_sensitive ( gboolean sensitive )
{
    gchar * items[] = {
        "/menubar/FileMenu/Save",
        "/menubar/FileMenu/SaveAs",
        "/menubar/FileMenu/DebugFile",
        "/menubar/FileMenu/Obfuscate",
        "/menubar/FileMenu/DebugMode",
        "/menubar/FileMenu/ExportFile",
        "/menubar/FileMenu/CreateArchive",
        "/menubar/FileMenu/ExportArchive",
        "/menubar/FileMenu/Close",
        "/menubar/EditMenu/NewTransaction",
        "/menubar/EditMenu/RemoveTransaction",
        "/menubar/EditMenu/TemplateTransaction",
        "/menubar/EditMenu/CloneTransaction",
        "/menubar/EditMenu/EditTransaction",
        "/menubar/EditMenu/ConvertToScheduled",
        "/menubar/EditMenu/MoveToAnotherAccount",
        "/menubar/EditMenu/Preferences",
        "/menubar/EditMenu/RemoveAccount",
        "/menubar/EditMenu/NewAccount",
        "/menubar/ViewMenu/ShowTransactionForm",
        "/menubar/ViewMenu/ShowReconciled",
        "/menubar/ViewMenu/ShowArchived",
        "/menubar/ViewMenu/ShowClosed",
        "/menubar/ViewMenu/ShowOneLine",
        "/menubar/ViewMenu/ShowTwoLines",
        "/menubar/ViewMenu/ShowThreeLines",
        "/menubar/ViewMenu/ShowFourLines",
        "/menubar/ViewMenu/InitwidthCol",
        NULL
    };
    gchar ** tmp = items;

    devel_debug_int (sensitive);

    while ( *tmp )
    {
        gsb_gui_sensitive_menu_item ( *tmp, sensitive );
        tmp++;
    }

    /* As this function may only be called when a new account is
     * created and the like, it is unlikely that we want to sensitive
     * transaction-related menus. */
    gsb_gui_sensitive_menu_item ( "/menubar/EditMenu/NewTransaction", FALSE );
    gsb_menu_set_menus_select_transaction_sensitive ( FALSE );
}

/**
 * Initialise la barre de menus si un compte est sélectionné
 *
 * \param sensitif
 *
 * \return
 * */
void gsb_menu_set_menus_view_account_sensitive ( gboolean sensitive )
{
    gchar * items[] = {
        "/menubar/ViewMenu/ShowTransactionForm",
        "/menubar/ViewMenu/ShowReconciled",
        "/menubar/ViewMenu/ShowArchived",
        "/menubar/ViewMenu/ShowOneLine",
        "/menubar/ViewMenu/ShowTwoLines",
        "/menubar/ViewMenu/ShowThreeLines",
        "/menubar/ViewMenu/ShowFourLines",
        "/menubar/ViewMenu/InitwidthCol",
        NULL
    };
    gchar **tmp = items;

    devel_debug_int (sensitive);

    while ( *tmp )
    {
        gsb_gui_sensitive_menu_item ( *tmp, sensitive );
        tmp++;
    }
}


/**
 *
 *
 * \param
 *
 * \return
 * */
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
