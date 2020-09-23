/*
 *  Laurent LEQUIEVRE
 *  Research Engineer, CNRS (France)
 *  Institut Pascal UMR6602
 *  laurent.lequievre@uca.fr
 */

#include "rqt_plugins/ur_controller_manager.h"
#include <pluginlib/class_list_macros.h>

#include <controller_manager_msgs/ListControllers.h>
#include <controller_manager_msgs/SwitchController.h>

#include <QMenu>

namespace ur_controller_plugins_ns {
	
  ControllerManagerPlugin::ControllerManagerPlugin(): rqt_gui_cpp::Plugin(), widget_(new QWidget()), vlayout_outer_(new QVBoxLayout()), tree_controllers_widget_(new QTreeWidget())
  {
	setObjectName("Plugin Controller Manager");  
  }
  
  void ControllerManagerPlugin::initPlugin(qt_gui_cpp::PluginContext& context)
  {
	    getNodeHandle().param<std::string>("/rqt_plugins/parameters/prefix_name_space", prefix_name_space_controllers_, "/campero");
	    
	    #if TRACE_ControllerManagerPlugin_ACTIVATED
			ROS_INFO_STREAM("Read from rosparam -> prefix_name_space = " << prefix_name_space_controllers_);
		#endif
	    
	    setupROSComponents_();
	    
		widget_->setWindowTitle("Controller Manager");
		
		vlayout_outer_->setObjectName("vertical_layout_outer");
		
		tree_controllers_widget_ ->setObjectName("tree_controllers_widget_");
		
		column_names_list_ << "name" << "state" << "type" << "hw_iface" << "resources";
		column_names_pretty_list_ << "Controller Name" << "State" << "Type" << "HW Interface" << "Claimed Resources";
		
		tree_controllers_widget_->setColumnCount(column_names_list_.size());
		tree_controllers_widget_->setHeaderLabels(column_names_pretty_list_);
		tree_controllers_widget_->sortByColumn(0, Qt::AscendingOrder);
		tree_controllers_widget_->setContextMenuPolicy(Qt::CustomContextMenu);
		
		connect(tree_controllers_widget_.get(), SIGNAL( customContextMenuRequested( const QPoint& ) ),
             this, SLOT( tree_controllers_widget_ContextMenu( const QPoint& ) ) );
		
		vlayout_outer_->addWidget(tree_controllers_widget_.get());
		
		widget_->setLayout(vlayout_outer_.get());
		
		context.addWidget(widget_.get());
		
		updateListControllers_();
  }
  
  void ControllerManagerPlugin::tree_controllers_widget_ContextMenu(const QPoint& aPoint)
  {
		QTreeWidgetItem* itemSelected = tree_controllers_widget_->itemAt(aPoint);
		
		if (itemSelected)
		{
		  QString state = itemSelected->text(1);
		  QString name = itemSelected->text(0);
		  
		  #if TRACE_ControllerManagerPlugin_ACTIVATED
			ROS_INFO("name = %s, state = %s",name.toStdString().c_str(), state.toStdString().c_str());
		  #endif
		  
		  QMenu menu(tree_controllers_widget_.get());
		  QIcon icon_start(QString::fromUtf8(":/icons/media-playback-start.png"));
		  QIcon icon_stop(QString::fromUtf8(":/icons/media-playback-stop.png"));
		  QAction* action;
		  
		  bool to_start = false;
		
		  if (state == "running")
		  {
			action = menu.addAction(icon_stop,"Stop Controller");
			to_start = false;
		  }
		  else
		    if (state == "stopped")
		    {
			  action = menu.addAction(icon_start,"Start Controller");
			  to_start = true;
		    }
		
		  QAction* action_menu_selected = menu.exec(tree_controllers_widget_->mapToGlobal(aPoint));
		
		  if (action_menu_selected)
		  {
		
			if (to_start)
			{
				#if TRACE_ControllerManagerPlugin_ACTIVATED
					ROS_INFO("action_start !");
				#endif
				switchController_(name, ActionController::START);
			}
			else
			{
				#if TRACE_ControllerManagerPlugin_ACTIVATED
					ROS_INFO("action_stop !");
				#endif
				switchController_(name, ActionController::STOP);
			}
	     }
	   }
   }
  
  void ControllerManagerPlugin::shutdownPlugin()
  {
	  #if TRACE_ControllerManagerPlugin_ACTIVATED
					ROS_INFO("shutdownPlugin !");
	  #endif
				
	  shutdownROSComponents_();
	  
	  disconnect(tree_controllers_widget_.get(), SIGNAL( customContextMenuRequested( const QPoint& ) ),
             this, SLOT( tree_controllers_widget_ContextMenu( const QPoint& ) ) );
	  
	  vlayout_outer_->removeWidget(tree_controllers_widget_.get());
  }
    
  void ControllerManagerPlugin::saveSettings(qt_gui_cpp::Settings& plugin_settings,
								qt_gui_cpp::Settings& instance_settings) const
  {
		// TODO save intrinsic configuration, usually using:
		// instance_settings.setValue(k, v)
  }

  void ControllerManagerPlugin::restoreSettings(const qt_gui_cpp::Settings& plugin_settings, 
								   const qt_gui_cpp::Settings& instance_settings)
  {
		// TODO restore intrinsic configuration, usually using:
		// v = instance_settings.value(k)
  }
  
  
  void ControllerManagerPlugin::setupROSComponents_()
  {
	  #if TRACE_ControllerManagerPlugin_ACTIVATED
		 ROS_INFO("setupROSComponents_ !");
	  #endif
	  
	  list_service_client_ = getNodeHandle().serviceClient<controller_manager_msgs::ListControllers>(prefix_name_space_controllers_ + "/controller_manager/list_controllers");
      switch_service_client_ = getNodeHandle().serviceClient<controller_manager_msgs::SwitchController>(prefix_name_space_controllers_ + "/controller_manager/switch_controller");
  }
  
  void ControllerManagerPlugin::shutdownROSComponents_()
  {
	  #if TRACE_ControllerManagerPlugin_ACTIVATED
		 ROS_INFO("shutdownROSComponents_ !");
	  #endif
	  
	  list_service_client_.shutdown();
	  switch_service_client_.shutdown();
  }
  
  void ControllerManagerPlugin::updateListControllers_()
  {
		#if TRACE_ControllerManagerPlugin_ACTIVATED
			ROS_INFO("ControllerManagerPlugin::updateListControllers_() !");
		#endif

		controller_manager_msgs::ListControllers controller_list;
	
		list_service_client_.call(controller_list);
		
		#if TRACE_ControllerManagerPlugin_ACTIVATED
		   ROS_INFO_STREAM("ControllerManagerPlugin::updateListControllers_() size of controller_list : " << controller_list.response.controller.size());
		#endif
		
		QStringList strListRessources;
		QStringList strListHWInterfaces;
		
		tree_controllers_widget_->clear();
	
		for (unsigned int i=0;i<controller_list.response.controller.size() ;i++)
		{
			// Create a new item
			QTreeWidgetItem* new_item = new QTreeWidgetItem(tree_controllers_widget_.get());
			new_item->setText(0, controller_list.response.controller[i].name.c_str());
			new_item->setText(1, controller_list.response.controller[i].state.c_str());
			new_item->setText(2, controller_list.response.controller[i].type.c_str());

			strListHWInterfaces.clear();
			strListRessources.clear();

			for (unsigned int j=0;j<controller_list.response.controller[i].claimed_resources.size() ;j++)
			{
				strListHWInterfaces << controller_list.response.controller[i].claimed_resources[j].hardware_interface.c_str();
				for (unsigned int k=0;k<controller_list.response.controller[i].claimed_resources[j].resources.size() ;k++)
				{
					strListRessources << controller_list.response.controller[i].claimed_resources[j].resources[k].c_str();
				}
			}

			new_item->setText(3, strListHWInterfaces.join(","));
			new_item->setText(4, strListRessources.join(","));	
		}
    }
    
    void ControllerManagerPlugin::switchController_(QString & name, ActionController action)
	{
		#if TRACE_ControllerManagerPlugin_ACTIVATED
			ROS_INFO("switchController_ !");
		#endif
		
		controller_manager_msgs::SwitchController switch_controller;
		switch_controller.request.start_controllers.clear();
		switch_controller.request.stop_controllers.clear();
		
		if (action == ActionController::START)
		{
			#if TRACE_ControllerManagerPlugin_ACTIVATED
				ROS_INFO("switch controller Start !");
			#endif
			switch_controller.request.start_controllers.push_back(name.toStdString().c_str());
		}
		else
			if (action == ActionController::STOP)
			{
				#if TRACE_ControllerManagerPlugin_ACTIVATED
					ROS_INFO("switch controller Stop !");
				#endif
				switch_controller.request.stop_controllers.push_back(name.toStdString().c_str());
			}
		switch_controller.request.strictness = controller_manager_msgs::SwitchControllerRequest::STRICT;	
		switch_service_client_.call(switch_controller);
		
		updateListControllers_();
    }
}

PLUGINLIB_DECLARE_CLASS(ur_controller_plugins_ns, ControllerManagerPlugin, ur_controller_plugins_ns::ControllerManagerPlugin, rqt_gui_cpp::Plugin)
