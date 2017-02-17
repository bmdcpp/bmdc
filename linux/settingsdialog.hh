/*
 * Copyright © 2004-2014 Jens Oknelid, paskharen@gmail.com
 * Copyright © 2014-2017 BMDC++
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * In addition, as a special exception, compiling, linking, and/or
 * using OpenSSL with this program is allowed.
 */

#ifndef _BMDC_SETTINGS_DIALOG_HH
#define _BMDC_SETTINGS_DIALOG_HH

#include "../dcpp/stdinc.h"
#include "../dcpp/DCPlusPlus.h"
#include "../dcpp/SettingsManager.h"
#include "../dcpp/UserCommand.h"
#include "../dcpp/HighlightManager.h"
#include "../dcpp/PluginDefs.h"
#if 0
#include "../dcpp/PluginManager.h"
#endif
#include "WulforUtil.hh"
#include "dialogentry.hh"
#include "treeview.hh"

class WulforSettingsManager;

class Settings:
	public DialogEntry
{
	public:
		Settings(GtkWindow* parent = NULL);
		~Settings();

		void response_gui();

	private:
		// GUI functions
		void addOption_gui(GtkListStore *store, const std::string &name, dcpp::SettingsManager::IntSetting setting);
		void addOption_gui(GtkListStore *store, char *name, dcpp::SettingsManager::BoolSetting setting);
		void addOption_gui(GtkListStore *store, WulforSettingsManager *wsm, const std::string &name,
			const std::string &key1, const std::string &key2, const std::string &key3, const std::string &key4);
		void addOption_gui(GtkListStore *store, WulforSettingsManager *wsm,
			const std::string &name, const std::string &key1, const std::string &key2,
			const std::string &key3, const int key4);
		void addOption_gui(GtkListStore *store, const std::string &name, const std::string &setting);
		//NOTE: BMDC++
		void addOption_gui(GtkListStore *store, const std::string &name, const std::string &setting, const std::string &backSetting);
		void addPreviewUL_gui(GtkListStore *store, const std::string &name, const std::string &color, const std::string &icon, const std::string &back);
		void addOption_gui_tabs(GtkListStore *store, const std::string &type, const std::string &key, const std::string &icon);
		//end
		void addOption_gui(GtkListStore *store, WulforSettingsManager *wsm, const std::string &name,
			const std::string &key1, const std::string &key2);
		void addOption_gui(GtkListStore *store, WulforSettingsManager *wsm, GtkIconTheme *iconTheme,
			const std::string &name, const std::string &key1);
		void addOption_gui(GtkListStore *store, WulforSettingsManager *wsm, GtkIconTheme *iconTheme,
			const std::string &name, const std::string &key1, const std::string &key2);
		void addOption_gui(GtkListStore *store, const std::string &type, const dcpp::StringList &exts,
			bool predefined, const int key);//NOTE: core 0.770
		void createOptionsView_gui(TreeView &treeView, GtkListStore *&store, const std::string &widgetName);
		void saveOptionsView_gui(TreeView &treeView, dcpp::SettingsManager *sm);

		void initPersonal_gui();
		void initConnection_gui();
		void initDownloads_gui();
		void initSharing_gui();
		void initAppearance_gui();
		void initLog_gui();
		void initAdvanced_gui();
		void initBandwidthLimiting_gui();//NOTE: core 0.762
		void initSearchTypes_gui();//NOTE: core 0.770
		void initHighlighting_gui();//NOTE: BMDC++
		#if 0
		void initPlugins_gui();//NOTE: BMDC++
		#endif
		void addShare_gui(std::string path, std::string name);
		void selectTextColor_gui(const int select);
		void selectTextStyle_gui(const int select);
		void loadUserCommands_gui();
		void saveUserCommand(dcpp::UserCommand *uc);
		void updateUserCommandTextSent_gui();
		bool validateUserCommandInput(const std::string &oldName = "");
		void showErrorDialog(const std::string error);
		void updateShares_gui();//NOTE: core 0.762
		void addSearchType_gui();//NOTE: core 0.770
		void modSearchType_gui();//NOTE: core 0.770
		void addExtension_gui(const std::string ext);//NOTE: core 0.770
		void showExtensionDialog_gui(bool add);//NOTE: core 0.770
		void setBgColorUserList();//NOTE: BMDC++

		// GUI callbacks
		static void onOptionsViewToggled_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data);
		static void onInDirect_gui(GtkToggleButton *button, gpointer data);
		static void onInFW_UPnP_gui(GtkToggleButton *button, gpointer data);
		static void onInPassive_gui(GtkToggleButton *button, gpointer data);
		static void onInFW_NAT_gui(GtkToggleButton *button, gpointer data);
		static void onOutDirect_gui(GtkToggleButton *button, gpointer data);
		static void onSocks5_gui(GtkToggleButton *button, gpointer data);
		static void onToggleAutoDetection(GtkWidget *widget, gpointer data);
		static void onBrowseFinished_gui(GtkWidget *widget, gpointer data);
		static void onBrowseUnfinished_gui(GtkWidget *widget, gpointer data);
		static void onPublicHubs_gui(GtkWidget *widget, gpointer data);
		static void onPublicAdd_gui(GtkWidget *widget, gpointer data);
		static void onPublicMoveUp_gui(GtkWidget *widget, gpointer data);
		static void onPublicMoveDown_gui(GtkWidget *widget, gpointer data);
		static void onPublicEdit_gui(GtkCellRendererText *cell, char *path, char *text, gpointer data);
		static void onPublicRemove_gui(GtkWidget *widget, gpointer data);
		static void onAddFavorite_gui(GtkWidget *widget, gpointer data);
		static void onRemoveFavorite_gui(GtkWidget *widget, gpointer data);
		static gboolean onFavoriteButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static void onAddShare_gui(GtkWidget *widget, gpointer data);
		static void onRemoveShare_gui(GtkWidget *widget, gpointer data);
		static gboolean onShareButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean onShareHiddenPressed_gui(GtkToggleButton *button, gpointer data);
		///@todo Uncomment when implemented
		//static void onWinColorClicked_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data);
		//static void onDownColorClicked_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data);
		//static void onUpColorClicked_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data);
		//static void onTextStyleClicked_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data);
		static void onLogBrowseClicked_gui(GtkWidget *widget, gpointer data);
		static void onLogMainClicked_gui(GtkToggleButton *button, gpointer data);
		static void onLogPrivateClicked_gui(GtkToggleButton *button, gpointer data);
		static void onLogDownloadClicked_gui(GtkToggleButton *button, gpointer data);
		static void onLogUploadClicked_gui(GtkToggleButton *button, gpointer data);
		static void onUserCommandAdd_gui(GtkWidget *widget, gpointer data);
		static void onUserCommandEdit_gui(GtkWidget *widget, gpointer data);
		static void onUserCommandMoveUp_gui(GtkWidget *widget, gpointer data);
		static void onUserCommandMoveDown_gui(GtkWidget *widget, gpointer data);
		static void onUserCommandRemove_gui(GtkWidget *widget, gpointer data);
		static void onUserCommandTypeSeparator_gui(GtkWidget *widget, gpointer data);
		static void onUserCommandTypeRaw_gui(GtkWidget *widget, gpointer data);
		static void onUserCommandTypeChat_gui(GtkWidget *widget, gpointer data);
		static void onUserCommandTypePM_gui(GtkWidget *widget, gpointer data);

		static gboolean onUserCommandKeyPress_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);
		static void onCertificatesPrivateBrowseClicked_gui(GtkWidget *widget, gpointer data);
		static void onCertificatesFileBrowseClicked_gui(GtkWidget *widget, gpointer data);
		static void onCertificatesPathBrowseClicked_gui(GtkWidget *widget, gpointer data);
		static void onGenerateCertificatesClicked_gui(GtkWidget *widget, gpointer data);
		static void onPreviewAdd_gui(GtkWidget *widget, gpointer data);
		static void onPreviewRemove_gui(GtkWidget *widget, gpointer data);
		static void onPreviewApply_gui(GtkWidget *widget, gpointer data);
		static void onPreviewKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);
		static void onPreviewButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static void onSoundFileBrowseClicked_gui(GtkWidget *widget, gpointer data);
		static void onSoundPlayButton_gui(GtkWidget *widget, gpointer data);
		static void onTextColorForeClicked_gui(GtkWidget *widget, gpointer data);
		static void onTextColorBackClicked_gui(GtkWidget *widget, gpointer data);
		static void onTextColorBWClicked_gui(GtkWidget *widget, gpointer data);
		static void onTextStyleClicked_gui(GtkWidget *widget, gpointer data);
		static void onTextStyleDefaultClicked_gui(GtkWidget *widget, gpointer data);
		static void onNotifyTestButton_gui(GtkWidget *widget, gpointer data);
		static void onNotifyIconFileBrowseClicked_gui(GtkWidget *widget, gpointer data);
		static void onNotifyKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);
		static void onNotifyButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static void onNotifyOKClicked_gui(GtkWidget *widget, gpointer data);
		static void onNotifyIconNoneButton_gui(GtkWidget *widget, gpointer data);
		static void onNotifyDefaultButton_gui(GtkWidget *widget, gpointer data);
		static void onImportThemeButton_gui(GtkWidget *widget, gpointer data);
		static void onExportThemeButton_gui(GtkWidget *widget, gpointer data);
		static void onDefaultIconsThemeButton_gui(GtkWidget *widget, gpointer data);
		static void onSystemIconsThemeButton_gui(GtkWidget *widget, gpointer data);
		static void onDefaultThemeButton_gui(GtkWidget *widget, gpointer data);
		static void onDefaultColorsSPButton_gui(GtkWidget *widget, gpointer data);
		static void onDefaultFrameSPButton_gui(GtkWidget *widget, gpointer data);
		static void onLimitSecondToggled_gui(GtkWidget *widget, gpointer data);//NOTE: core 0.762
		static void onAddSTButton_gui(GtkWidget *widget, gpointer data);//NOTE: core 0.770
		static void onModifySTButton_gui(GtkWidget *widget, gpointer data);//NOTE: core 0.770
		static void onRenameSTButton_gui(GtkWidget *widget, gpointer data);//NOTE: core 0.770
		static void onRemoveSTButton_gui(GtkWidget *widget, gpointer data);//NOTE: core 0.770
		static void onDefaultSTButton_gui(GtkWidget *widget, gpointer data);//NOTE: core 0.770
		static void onAddExtensionButton_gui(GtkWidget *widget, gpointer data);//NOTE: core 0.770
		static void onEditExtensionButton_gui(GtkWidget *widget, gpointer data);//NOTE: core 0.770
		static void onRemoveExtensionButton_gui(GtkWidget *widget, gpointer data);//NOTE: core 0.770
		static void onUpExtensionButton_gui(GtkWidget *widget, gpointer data);//NOTE: core 0.770
		static void onDownExtensionButton_gui(GtkWidget *widget, gpointer data);//NOTE: core 0.770
		static void onSTKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);//NOTE: core 0.770
		static void onSTButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);//NOTE: core 0.770
		static void onPictureShare_gui(GtkWidget *widget, gpointer data);
		//BMDC++
		static void onTextColorForeULClicked_gui(GtkWidget *widget, gpointer data);
		static void onTextColorDefaultULClicked_gui(GtkWidget *widget, gpointer data);
		static void onBgColorULClicked_gui(GtkWidget *widget, gpointer data);
		static void onAddHighlighting_gui(GtkWidget *widget, gpointer data);
		static void onEditHighlighting_gui(GtkWidget *widget, gpointer data);
		static void onRemoveHighlighting_gui(GtkWidget *widget, gpointer data);
		static void onColorText_gui(GtkWidget *widget, gpointer data);
		static void onColorBack_gui(GtkWidget *widget, gpointer data);
		static void onSetBackGroundChat(GtkWidget *widget , gpointer data);
		static void onSound_gui(GtkWidget *widget, gpointer data);
		static void onToggledHGText_gui(GtkWidget *widget, gpointer data);
		static void onToggledHGSound_gui(GtkWidget *widget, gpointer data);
		static void onToggledHGColor_gui(GtkWidget *widget, gpointer data);
		static void onToggledHGNotify_gui(GtkWidget *widget, gpointer data);
		static void onRawsClicked_gui(GtkToggleButton *button, gpointer data );
		//Plugins
		#if 0
		static void onAddPluginTo_gui(GtkWidget *widget, gpointer data);
		static void onRemPluginFrom_gui(GtkWidget *widget, gpointer data);
		static void onConfigurePlugin_gui(GtkWidget *widget, gpointer data);
		static void onAboutPlugin_gui(GtkWidget *widget, gpointer data);
		#endif
		//BMDC
		static void onForeColorChooserTab(GtkWidget *button, gpointer data) ;
		static void onBackColorChooserTab(GtkWidget *button, gpointer data) ;
		static void onForeColorChooserTab_unread(GtkWidget *button, gpointer data) ;
		static void onBackColorChooserTab_unread(GtkWidget *button, gpointer data) ;
		static void onChangeTabSelections(GtkTreeSelection *selection, gpointer data);
		void changeTab(GtkTreeSelection *selection);
		//BMDC
		static void makeColor(GtkTreeViewColumn *column,GtkCellRenderer *cell, GtkTreeModel *model, GtkTreeIter *iter,gpointer data);
		// Client functions
		void saveSettings_client();
		void shareHidden_client(bool show);
		void addShare_client(std::string path, std::string name);
		void removeUserCommand_client(std::string name, std::string hub);
		void moveUserCommand_client(std::string name, std::string hub, int pos);
		void generateCertificates_client();
		void setColorUL();//BMDC++
		void setDefaultColor(std::string color, std::string name, GtkTreeIter *iter);//BMDC++
		void saveHighlighting(dcpp::StringMap &params, bool add, const std::string &name = "");//BMDC++
		void addHighlighting_to_gui(dcpp::ColorSettings &cs, bool add);//BMDC++
		//Plugins
		#if 0
		void addToGuiPlg();
		void RemovePlg_client(std::string sel);
		static void onToggledPluginsClicked_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data);
		#endif

		GtkComboBoxText *connectionSpeedComboBox;
		GtkListStore *downloadToStore, *publicListStore, *queueStore,
			*shareStore, *appearanceStore, *tabStore, *windowStore1,
			*windowStore2, *windowStore3, *advancedStore, *certificatesStore, *userCommandStore,
			*previewAppToStore, *soundStore, *textStyleStore, *notifyStore, *themeIconsStore,
			*toolbarStore, *extensionStore, *searchTypeStore, *userListStore1, *userListStore2, *hStore, *plStore;
		GtkListStore *tabColorStore;	
		TreeView downloadToView, publicListView, queueView, shareView,
			appearanceView, tabView, windowView1, windowView2,
			windowView3, advancedView, certificatesView, userCommandView,
			previewAppView, soundView, textStyleView, notifyView, themeIconsView,
			toolbarView, extensionView, searchTypeView, userListNames, userListPreview, hView, plView;
		TreeView tabsColors;	
		GtkTextBuffer *textStyleBuffer;
		GtkTreeSelection *selection, *plselection, *tabSelections;

		typedef std::map<std::string, int> IntMap;
		typedef std::map<std::string, std::string> StringMap;

		IntMap defaultIntTheme;
		IntMap intMapTheme;
		StringMap stringMapTheme;
		StringMap defaultStringTheme;
		UnMapIter colorsIters;

		bool loadFileTheme(const std::string &file);
		void saveFileTheme(const std::string &file);
		void saveTheme();
		void setTheme();
		int getIntTheme(const std::string &key, bool useDefault = FALSE);
		std::string getStringTheme(const std::string &key, bool useDefault = FALSE);
		void set(const std::string &key, int value);
		void set(const std::string &key, const std::string &value);
		void applyIconsTheme(bool useDefault = FALSE);
		void applyTextTheme(bool useDefault = FALSE);

		dcpp::ColorList pList;
		gboolean isSensitiveHG[4];
		void setColorRow(std::string cell);
};

#else
class Settings;
#endif
