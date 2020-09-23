/*
 *  Laurent LEQUIEVRE
 *  Research Engineer, CNRS (France)
 *  Institut Pascal UMR6602
 *  laurent.lequievre@uca.fr
 */

#ifndef controller_manager_plugin_H
#define controller_manager_plugin_H

#include "ros/ros.h"
#include <rqt_gui_cpp/plugin.h>
#include <memory>
#include <string>

#include <QWidget>
#include <QVBoxLayout>
#include <QTreeWidget>

#define TRACE_ControllerManagerPlugin_ACTIVATED 1

namespace ur_controller_plugins_ns {
	
	class ControllerManagerPlugin : public rqt_gui_cpp::Plugin
	{
		Q_OBJECT
		
		enum ActionController { START, STOP };
		
		public:

			ControllerManagerPlugin();

			virtual void initPlugin(qt_gui_cpp::PluginContext& context);

			virtual void shutdownPlugin();

			virtual void saveSettings(qt_gui_cpp::Settings& plugin_settings, 
									  qt_gui_cpp::Settings& instance_settings) const;

			virtual void restoreSettings(const qt_gui_cpp::Settings& plugin_settings, 
										 const qt_gui_cpp::Settings& instance_settings);
			
	     public slots:
		 
			void tree_controllers_widget_ContextMenu(const QPoint& aPoint);		
										 
		 private:
		    std::shared_ptr<QWidget> widget_;
		    std::shared_ptr<QVBoxLayout> vlayout_outer_;
		    std::shared_ptr<QTreeWidget> tree_controllers_widget_;
		    
		    QStringList column_names_list_, column_names_pretty_list_;
		    
		    ros::ServiceClient list_service_client_;
		    ros::ServiceClient switch_service_client_;
		    
		    void setupROSComponents_();
			void shutdownROSComponents_();
			
			void updateListControllers_();
			void switchController_(QString & name, ActionController action);
			
			std::string prefix_name_space_controllers_;
		
	}; // End of class
	
} // End of namespace

#endif
