// file gui_BM.c
#include "bismon.h"

GtkWidget *mainwin_BM;

GtkTextTagTable *browsertagtable_BM;
GtkTextBuffer *browserbuf_BM;
GtkWidget *browserview_BM;

GtkTextTagTable *commandtagtable_BM;
GtkTextBuffer *commandbuf_BM;
GtkWidget *commandview_BM;

GtkTextTagTable *logtagtable_BM;
GtkTextBuffer *logbuf_BM;
GtkWidget *logview_BM;

GtkWidget *msglab_BM;

const char *
gobjectclassnamedbg_BM (GObject * ptr)
{
  if (!ptr)
    return "*nil*";
  return G_OBJECT_CLASS_NAME (G_OBJECT_GET_CLASS (ptr));
}                               /* end gobjectclassnamedbg_BM */

void quit_BM (void);
void
quit_BM (void)
{
  //  printf ("quit_BM\n");
  GtkWidget *quitdialog =       //
    gtk_message_dialog_new_with_markup  //
    (GTK_WINDOW (mainwin_BM),
     GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
     GTK_MESSAGE_QUESTION,
     GTK_BUTTONS_OK_CANCEL,
     "Quit without dumping?");
  int resp = gtk_dialog_run (GTK_DIALOG (quitdialog));
  if (resp == GTK_RESPONSE_OK)
    gtk_main_quit ();
  gtk_widget_destroy (quitdialog);
}                               /* end quit_BM */

void exit_BM (void);
void
exit_BM (void)
{
  extern char *dump_dir_bm;
  dump_BM (dump_dir_bm, NULL);
  gtk_main_quit ();
}                               /* end exit_BM */

void do_dump_BM (void);
void
do_dump_BM (void)
{
  extern char *dump_dir_bm;
  dump_BM (dump_dir_bm, NULL);
}                               /* end do_dump_BM */

bool deletemainwin_BM (GtkWidget *, GdkEvent *, gpointer);
bool
deletemainwin_BM (GtkWidget * widget __attribute__ ((unused)),
                  GdkEvent * ev __attribute__ ((unused)),
                  gpointer data __attribute__ ((unused)))
{
  // printf ("deletemainwin_BM\n");
  GtkWidget *quitdialog =       //
    gtk_message_dialog_new_with_markup  //
    (GTK_WINDOW (mainwin_BM),
     GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
     GTK_MESSAGE_QUESTION,
     GTK_BUTTONS_OK_CANCEL,
     "Quit (after delete) without dumping?");
  int resp = gtk_dialog_run (GTK_DIALOG (quitdialog));
  if (resp == GTK_RESPONSE_OK)
    {
      gtk_main_quit ();
      gtk_widget_destroy (quitdialog);
      return false;             // to let the window be destroyed
    }
  else
    {
      gtk_widget_destroy (quitdialog);
      return true;
    }
}                               /* end deletemainwin_BM */





void
initialize_gui_BM (const char *builderfile)
{
  if (!builderfile)
    builderfile = "bismon.ui";
  GtkBuilder *bld = gtk_builder_new_from_file (builderfile);
  //gtk_builder_add_callback_symbols (bld, "quitaction_BM", quit_BM, NULL);
  mainwin_BM = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  browsertagtable_BM =          //
    GTK_TEXT_TAG_TABLE (gtk_builder_get_object (bld, "browsertagtable_id"));
  commandtagtable_BM =          //
    GTK_TEXT_TAG_TABLE (gtk_builder_get_object (bld, "commandtagtable_id"));
  logtagtable_BM =              //
    GTK_TEXT_TAG_TABLE (gtk_builder_get_object (bld, "logtagtable_id"));
  GtkWidget *mainvbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
  gtk_container_add (GTK_CONTAINER (mainwin_BM), mainvbox);
  GtkWidget *mainmenubar = gtk_menu_bar_new ();
  gtk_box_pack_start (GTK_BOX (mainvbox), mainmenubar, /*expand= */ false,
                      /*fill= */ false, 2);
  GtkWidget *appquit =
    GTK_WIDGET (gtk_builder_get_object (bld, "appquit_id"));
  g_signal_connect (appquit, "activate", quit_BM, NULL);
  g_signal_connect (mainwin_BM, "delete-event", (GCallback) deletemainwin_BM,
                    NULL);
  GtkWidget *appexit =
    GTK_WIDGET (gtk_builder_get_object (bld, "appexit_id"));
  g_signal_connect (appexit, "activate", exit_BM, NULL);
  GtkWidget *appdump =
    GTK_WIDGET (gtk_builder_get_object (bld, "appdump_id"));
  g_signal_connect (appdump, "activate", do_dump_BM, NULL);
  GtkWidget *appmenu =
    GTK_WIDGET (gtk_builder_get_object (bld, "menuapp_id"));
  assert (GTK_IS_WIDGET (appmenu));
  gtk_menu_shell_append (GTK_MENU_SHELL (mainmenubar), appmenu);
  GtkWidget *editmenu =
    GTK_WIDGET (gtk_builder_get_object (bld, "menuedit_id"));
  assert (GTK_IS_WIDGET (editmenu));
  gtk_menu_shell_append (GTK_MENU_SHELL (mainmenubar), editmenu);
  GtkWidget *sep1 = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
  gtk_box_pack_start (GTK_BOX (mainvbox), sep1, /*expand= */ false,
                      /*fill= */ false, 2);
  GtkWidget *paned1 = gtk_paned_new (GTK_ORIENTATION_VERTICAL);
  gtk_paned_set_wide_handle (GTK_PANED (paned1), true);
  gtk_box_pack_start (GTK_BOX (mainvbox), paned1, /*expand= */ true,
                      /*fill= */ true, 2);
  browserbuf_BM = gtk_text_buffer_new (browsertagtable_BM);
  gtk_text_buffer_insert_at_cursor (browserbuf_BM, "//browser\n", -1);
  browserview_BM = gtk_text_view_new_with_buffer (browserbuf_BM);
  gtk_text_view_set_editable (GTK_TEXT_VIEW (browserview_BM), false);
  GtkWidget *browserscrolw = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (browserscrolw), browserview_BM);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (browserscrolw),
                                  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  //
  commandbuf_BM = gtk_text_buffer_new (commandtagtable_BM);
  commandview_BM = gtk_text_view_new_with_buffer (commandbuf_BM);
  gtk_text_view_set_editable (GTK_TEXT_VIEW (commandview_BM), true);
  gtk_widget_set_tooltip_markup
    (GTK_WIDGET (commandview_BM), "<big><b>command view</b></big>\n");
  gtk_text_buffer_insert_at_cursor (commandbuf_BM, "|command|\n", -1);
  GtkWidget *commandscrolw = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (commandscrolw), commandview_BM);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (commandscrolw),
                                  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  //
  logbuf_BM = gtk_text_buffer_new (logtagtable_BM);
  logview_BM = gtk_text_view_new_with_buffer (logbuf_BM);
  gtk_text_view_set_editable (GTK_TEXT_VIEW (logview_BM), false);
  gtk_text_buffer_insert_at_cursor (logbuf_BM, "*log*\n", -1);
  GtkWidget *logscrolw = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (logscrolw), logview_BM);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (logscrolw),
                                  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  //
  gtk_paned_add1 (GTK_PANED (paned1), browserscrolw);
  GtkWidget *paned2 = gtk_paned_new (GTK_ORIENTATION_VERTICAL);
  gtk_paned_set_wide_handle (GTK_PANED (paned2), true);
  gtk_paned_add2 (GTK_PANED (paned1), paned2);
  gtk_paned_add1 (GTK_PANED (paned2), commandscrolw);
  gtk_paned_add2 (GTK_PANED (paned2), logscrolw);
  msglab_BM = gtk_label_new ("-");
  gtk_box_pack_start (GTK_BOX (mainvbox), msglab_BM, /*expand= */ false,
                      /*fill= */ false, 3);
  gtk_window_set_title (GTK_WINDOW (mainwin_BM), "bismon");
  gtk_window_set_default_size (GTK_WINDOW (mainwin_BM), 560, 430);
  gtk_widget_show_all (GTK_WIDGET (mainwin_BM));
}                               /* end initialize_gui_BM */