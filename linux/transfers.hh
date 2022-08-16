/*
 * Copyright © 2004-2015 Jens Oknelid, paskharen@gmail.com
 * Copyright © 2011-2024 BMDC++
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

#ifndef _BMDC_TRANSFERS_HH
#define _BMDC_TRANSFERS_HH

#include "../dcpp/stdinc.h"
#include "../dcpp/DCPlusPlus.h"
#include "../dcpp/ConnectionManager.h"
#include "../dcpp/DownloadManager.h"
#include "../dcpp/LogManager.h"
#include "../dcpp/QueueManager.h"
#include "../dcpp/UploadManager.h"

#include "treeview.hh"
#include "entry.hh"
#include "sound.hh"

class SearchEntry;
class PreviewMenu;
class UserCommandMenu;

class Transfers:
	public dcpp::ConnectionManagerListener,
	public dcpp::DownloadManagerListener,
	public dcpp::QueueManagerListener,
	public dcpp::UploadManagerListener,
	public Entry
{
	public:
		Transfers();
		virtual ~Transfers();

		GtkWidget *getContainer() { return getWidget("mainBox"); }
		virtual void show();

	private:
		//Made clang happy
		using dcpp::ConnectionManagerListener::on;
		using dcpp::DownloadManagerListener::on;
		using dcpp::QueueManagerListener::on;
		using dcpp::UploadManagerListener::on;
		//End
		// GUI functions
		void addConnection_gui(dcpp::StringMap params, bool download);
		void removeConnection_gui(const std::string cid, bool download);

		void initTransfer_gui(dcpp::StringMap params);
		void updateTransfer_gui(dcpp::StringMap params, bool download, Sound::TypeSound sound);
		void updateFilePosition_gui(const std::string cid, int64_t filePosition);
		void updateParent_gui(GtkTreeIter* iter);
		void finishParent_gui(const std::string target, const std::string status, Sound::TypeSound sound);

		bool findParent_gui(const std::string& target, GtkTreeIter* iter);
		bool findTransfer_gui(const std::string& cid, bool download, GtkTreeIter* iter);

		void playSound_gui(Sound::TypeSound sound);

		// GUI callbacks
		static void on_inner_widget_right_btn_pressed (GtkGestureClick *gesture, int       n_press,
                                   double             x,
                                   double             y,
                                   gpointer         *data);

		static void on_inner_widget_right_btn_released (GtkGestureClick *gesture,int       n_press,
                                    double           x,
                                    double           y,
                                    GtkWidget       *widget);
		//static gboolean onTransferButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		//static gboolean onTransferButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		
		static void onGetFileListClicked_gui(GtkWidget *widget,GVariant  *value, gpointer data);
		static void onMatchQueueClicked_gui(GtkWidget *widget,GVariant  *value, gpointer data);
		static void onPrivateMessageClicked_gui(GtkWidget *widget,GVariant  *value, gpointer data);
		static void onAddFavoriteUserClicked_gui(GtkWidget *widget,GVariant  *value, gpointer data);
		static void onGrantExtraSlotClicked_gui(GtkWidget *widget,GVariant  *value, gpointer data);
		static void onRemoveUserFromQueueClicked_gui(GtkWidget *widget,GVariant  *value, gpointer data);
		static void onForceAttemptClicked_gui(GtkWidget *widget,GVariant  *value, gpointer data);
		static void onCloseConnectionClicked_gui(GtkWidget *widget,GVariant  *value, gpointer data);
		static void onSearchAlternateClicked_gui(GtkWidget *widget,GVariant  *value, gpointer data);

		// Client functions
		void getParams_client(dcpp::StringMap& params, dcpp::ConnectionQueueItem* cqi);
        // down = dowload , false mean upload
		void getParams_client(dcpp::StringMap& params, dcpp::Transfer* transfer , bool down = true);
		void getFileList_client(std::string cid, std::string hubUrl);
		void matchQueue_client(std::string cid, std::string hubUrl);
		void addFavoriteUser_client(std::string cid);
		void grantExtraSlot_client(std::string cid, std::string hubUrl);
		void removeUserFromQueue_client(std::string cid);
		void forceAttempt_client(std::string cid);
		void closeConnection_client(std::string cid, bool download);

		// DownloadManager
		virtual void on(dcpp::DownloadManagerListener::Requesting, dcpp::Download* dl) noexcept;
		virtual void on(dcpp::DownloadManagerListener::Starting, dcpp::Download* dl) noexcept;
		virtual void on(dcpp::DownloadManagerListener::Tick, const dcpp::DownloadList& dls) noexcept;
		virtual void on(dcpp::DownloadManagerListener::Complete, dcpp::Download* dl) noexcept;
		virtual void on(dcpp::DownloadManagerListener::Failed, dcpp::Download* dl, const std::string& reason) noexcept;
		// ConnectionManager
		virtual void on(dcpp::ConnectionManagerListener::Added, dcpp::ConnectionQueueItem* cqi) noexcept;
		virtual void on(dcpp::ConnectionManagerListener::Connected, dcpp::ConnectionQueueItem* cqi) noexcept;
		virtual void on(dcpp::ConnectionManagerListener::Removed, dcpp::ConnectionQueueItem* cqi) noexcept;
		virtual void on(dcpp::ConnectionManagerListener::Failed, dcpp::ConnectionQueueItem* cqi, const std::string&) noexcept;
		virtual void on(dcpp::ConnectionManagerListener::StatusChanged, dcpp::ConnectionQueueItem* cqi) noexcept;
		// QueueManager
		virtual void on(dcpp::QueueManagerListener::Finished, dcpp::QueueItem* qi, const std::string&, int64_t size) noexcept;
		virtual void on(dcpp::QueueManagerListener::Removed, dcpp::QueueItem* qi) noexcept;
		// UploadManager
		virtual void on(dcpp::UploadManagerListener::Starting, dcpp::Upload* ul) noexcept;
		virtual void on(dcpp::UploadManagerListener::Tick, const dcpp::UploadList& uls) noexcept;
		virtual void on(dcpp::UploadManagerListener::Complete, dcpp::Upload* ul) noexcept;
		virtual void on(dcpp::UploadManagerListener::Failed, dcpp::Upload* ul, const std::string& reason) noexcept;

		TreeView transferView;
		GtkTreeStore *transferStore;
		GtkTreeSelection *transferSelection;
		UserCommandMenu* userCommandMenu;
		PreviewMenu *appsPreviewMenu;

		static const GActionEntry win_entries[];
};

#endif // BMDC_TRANSFERS_HH

