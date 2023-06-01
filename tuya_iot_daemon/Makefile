include $(TOPDIR)/rules.mk

PKG_NAME:=tuya_iot_daemon
PKG_RELEASE:=1
PKG_VERSION:=1.0.0

include $(INCLUDE_DIR)/package.mk

define Package/tuya_iot_daemon
	CATEGORY:=Base system
	TITLE:=tuya_iot_daemon
	DEPENDS:=+libtuya_iot +libubus +libubox +libblobmsg-json +liblua
endef

define Package/tuya_iot_daemon/description
	Tuya IoT daemon package
endef

define Package/tuya_iot_daemon/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/local/lib/tuya_modules/
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tuya_iot_daemon $(1)/usr/bin
	$(INSTALL_BIN) ./files/tuya_daemon.init $(1)/etc/init.d/tuya_daemon
	$(INSTALL_CONF) ./files/tuya_daemon.config $(1)/etc/config/tuya_daemon
	$(INSTALL_BIN) ./files/tuya_modules/*.lua $(1)/usr/local/lib/tuya_modules/
endef

$(eval $(call BuildPackage,tuya_iot_daemon))