#ifndef WEBAPI_H
#define WEBAPI_H

#define APPLICATION_TAG                     "OpenTeraPlus"

// TERASERVER URL Definitions
#define WEB_LOGIN_PATH                      "/api/user/login"
#define WEB_LOGOUT_PATH                     "/api/user/logout"
#define WEB_USERINFO_PATH                   "/api/user/users"
#define WEB_USERGROUPINFO_PATH              "/api/user/usergroups"
#define WEB_USERUSERGROUPINFO_PATH          "/api/user/users/usergroups"
#define WEB_USERPREFSINFO_PATH              "/api/user/users/preferences"
#define WEB_SITEINFO_PATH                   "/api/user/sites"
#define WEB_PROJECTINFO_PATH                "/api/user/projects"
#define WEB_SITEACCESS_PATH                 "/api/user/siteaccess"
#define WEB_PROJECTACCESS_PATH              "/api/user/projectaccess"
#define WEB_DEVICEINFO_PATH                 "/api/user/devices"
#define WEB_DEVICESITEINFO_PATH             "/api/user/devicesites"
#define WEB_DEVICEPROJECTINFO_PATH          "/api/user/deviceprojects"
#define WEB_DEVICEPARTICIPANTINFO_PATH      "/api/user/deviceparticipants"
#define WEB_DEVICESUBTYPE_PATH              "/api/user/devicesubtypes"
#define WEB_DEVICETYPE_PATH                 "/api/user/devicetypes"
#define WEB_PARTICIPANTINFO_PATH            "/api/user/participants"
#define WEB_GROUPINFO_PATH                  "/api/user/groups"
#define WEB_SESSIONINFO_PATH                "/api/user/sessions"
#define WEB_SESSIONTYPE_PATH                "/api/user/sessiontypes"
#define WEB_SESSIONTYPEDEVICETYPE_PATH      "/api/user/sessiontypedevicetypes"
#define WEB_SESSIONTYPEPROJECT_PATH         "/api/user/sessiontypeprojects"
#define WEB_SESSIONEVENT_PATH               "/api/user/sessions/events"
#define WEB_SERVICEINFO_PATH                "/api/user/services"
#define WEB_SERVICEPROJECTINFO_PATH         "/api/user/services/projects"
#define WEB_SERVICEACCESSINFO_PATH          "/api/user/services/access"
#define WEB_SERVICECONFIGINFO_PATH          "/api/user/services/configs"
#define WEB_STATS_PATH                      "/api/user/stats"

#define WEB_SESSIONMANAGER_PATH             "/api/user/sessions/manager"

#define WEB_ONLINEUSERINFO_PATH             "/api/user/users/online"
#define WEB_ONLINEPARTICIPANTINFO_PATH      "/api/user/participants/online"
#define WEB_ONLINEDEVICEINFO_PATH           "/api/user/devices/online"

#define WEB_FORMS_PATH                  "/api/user/forms"
#define WEB_FORMS_QUERY_USER            "type=user"
#define WEB_FORMS_QUERY_SITE            "type=site"
#define WEB_FORMS_QUERY_DEVICE          "type=device"
#define WEB_FORMS_QUERY_PROJECT         "type=project"
#define WEB_FORMS_QUERY_GROUP           "type=group"
#define WEB_FORMS_QUERY_PARTICIPANT     "type=participant"
#define WEB_FORMS_QUERY_SESSION_TYPE    "type=session_type"
#define WEB_FORMS_QUERY_SESSION         "type=session"
#define WEB_FORMS_QUERY_DEVICE_SUBTYPE  "type=device_subtype"
#define WEB_FORMS_QUERY_DEVICE_TYPE     "type=device_type"
#define WEB_FORMS_QUERY_USER_GROUP      "type=user_group"
#define WEB_FORMS_QUERY_SERVICE         "type=service"
#define WEB_FORMS_QUERY_SERVICE_CONFIG  "type=service_config"

#define WEB_QUERY_UUID              "uuid"
#define WEB_QUERY_APPTAG            "app_tag"
#define WEB_QUERY_ID_USER           "id_user"
#define WEB_QUERY_ID                "id"
#define WEB_QUERY_ID_SITE           "id_site"
#define WEB_QUERY_ID_PARTICIPANT    "id_participant"
#define WEB_QUERY_ID_PROJECT        "id_project"
#define WEB_QUERY_ID_DEVICE         "id_device"
#define WEB_QUERY_ID_DEVICE_TYPE    "id_device_type"
#define WEB_QUERY_ID_DEVICE_SUBTYPE "id_device_subtype"
#define WEB_QUERY_ID_GROUP          "id_group"
#define WEB_QUERY_ID_SESSION        "id_session"
#define WEB_QUERY_ID_SESSION_TYPE   "id_session_type"
#define WEB_QUERY_ID_USER_GROUP     "id_user_group"
#define WEB_QUERY_ID_SERVICE        "id_service"
#define WEB_QUERY_ID_SPECIFIC       "id_specific"
#define WEB_QUERY_SERVICE_KEY       "service_key"
#define WEB_QUERY_SESSION_UUID      "session_uuid"

#define WEB_QUERY_LIST              "list"
#define WEB_QUERY_AVAILABLE         "available"
#define WEB_QUERY_DOWNLOAD          "download"
#define WEB_QUERY_ENABLED           "enabled"
#define WEB_QUERY_WITH_USERGROUPS   "with_usergroups"
#define WEB_QUERY_WITH_SITES        "with_sites"
#define WEB_QUERY_WITH_PROJECTS     "with_projects"
#define WEB_QUERY_WITH_ROLES        "with_roles"
#define WEB_QUERY_WITH_EMPTY        "with_empty"
#define WEB_QUERY_WITH_PARTICIPANTS "with_participants"
#define WEB_QUERY_WITH_SERVICES     "with_services"
#define WEB_QUERY_WITH_CONFIG       "with_config"
#define WEB_QUERY_WITH_STATUS       "with_status"
#define WEB_QUERY_WITH_BUSY         "with_busy"
#define WEB_QUERY_SELF              "self"
#define WEB_QUERY_BY_USERS          "by_users"
#define WEB_QUERY_NO_GROUP          "no_group"

#endif // WEBAPI_H
