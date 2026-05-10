// This is the main entry point for the backend server. It sets up the middleware
// that handles things like CORS, login sessions, and role-based access before
// any request reaches a controller.

#include <drogon/drogon.h>
#include <string>
#include <vector>
#include <algorithm>

// This struct and the PERMISSIONS array below define which roles are allowed to
// view (GET) or edit (POST) each API route.
struct RoutePermission {
    std::string pathPrefix;
    std::vector<std::string> viewRoles;   // GET access
    std::vector<std::string> editRoles;   // POST access
};

static const std::vector<RoutePermission> PERMISSIONS = {
    { "/api/patients", {"admin","doctor","nurse","front_desk"},  {"admin","front_desk"} },
    { "/api/orders",   {"admin","doctor","nurse","front_desk"},  {"admin","doctor","nurse"} },
    { "/api/labs",     {"admin","doctor","nurse","patient"},     {"admin","doctor","nurse"} },
    { "/api/tests",    {"admin","doctor","nurse","patient"},     {"admin","doctor","nurse"} },
    { "/api/billing",  {"admin","front_desk","patient"},         {"admin","front_desk"} },
};

// Checking if a role is in the allowed roles list.
static bool hasRole(const std::vector<std::string>& roles, const std::string& role)
{
    return std::find(roles.begin(), roles.end(), role) != roles.end();
}

// This controls which roles can load each HTML page. If a user doesn't have
// the right role, they get sent back to the login page.
struct PagePermission {
    std::string page;
    std::vector<std::string> viewRoles;
};

static const std::vector<PagePermission> PAGE_PERMISSIONS = {
    { "/admit.html",   {"admin","doctor","nurse","front_desk"} },
    { "/orders.html",  {"admin","doctor","nurse","front_desk"} },
    { "/labs.html",    {"admin","doctor","nurse","patient"} },
    { "/tests.html",   {"admin","doctor","nurse","patient"} },
    { "/billing.html", {"admin","front_desk","patient"} },
};

int main()
{
    drogon::app().loadConfigFile("config.json");

    // This middleware runs before every request hits a controller. It handles
    // CORS, checks if the user is logged in, and makes sure they have the right role.
    drogon::app().registerPreHandlingAdvice(
        [](const drogon::HttpRequestPtr &req,
           drogon::AdviceCallback &&callback,
           drogon::AdviceChainCallback &&chainCallback) {

            // Responding to CORS preflight requests from the browser.
            if (req->method() == drogon::Options) {
                auto resp = drogon::HttpResponse::newHttpResponse();
                resp->addHeader("Access-Control-Allow-Origin", "*");
                resp->addHeader("Access-Control-Allow-Methods", "GET,POST,OPTIONS");
                resp->addHeader("Access-Control-Allow-Headers", "Content-Type");
                callback(resp);
                return;
            }

            std::string path = req->path();

            // These routes don't need a login session since they handle logging in/registering.
            if (path == "/api/login" || path == "/api/register" || path == "/api/session" || path == "/api/dev-login") {
                chainCallback();
                return;
            }

            // If the request is for an HTML page, check if the user is logged in and has the right role.
            if (path.substr(0, 4) != "/api") {
                for (auto& pg : PAGE_PERMISSIONS)
                {
                    if (path == pg.page)
                    {
                        auto session = req->session();
                        auto role = session->getOptional<std::string>("role");

                        if (!role || !hasRole(pg.viewRoles, *role)) {
                            auto resp = drogon::HttpResponse::newRedirectionResponse("index.html");
                            callback(resp);
                            return;
                        }
                        break;
                    }
                }
                // Static files like CSS and JS don't need authentication.
                chainCallback();
                return;
            }

            // Everything past this point is an API route, so the user needs to be logged in.
            auto session = req->session();
            auto role = session->getOptional<std::string>("role");
            if (!role) {
                Json::Value err;
                err["error"] = "Not authenticated. Please log in.";
                auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
                resp->setStatusCode(drogon::k401Unauthorized);
                callback(resp);
                return;
            }

            // Any logged-in user can log out regardless of their role.
            if (path == "/api/logout") {
                chainCallback();
                return;
            }

            // Matching the request to a permission entry and checking if the user's role
            // is allowed. GET requests check view permissions, POST requests check edit permissions.
            for (auto& perm : PERMISSIONS)
            {
                if (path.substr(0, perm.pathPrefix.size()) == perm.pathPrefix)
                {
                    bool isWrite = (req->method() == drogon::Post);
                    const auto& allowed = isWrite ? perm.editRoles : perm.viewRoles;

                    if (!hasRole(allowed, *role)) {
                        Json::Value err;
                        err["error"] = "Access denied for your role.";
                        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
                        resp->setStatusCode(drogon::k403Forbidden);
                        callback(resp);
                        return;
                    }

                    chainCallback();
                    return;
                }
            }

            // If no permission entry matched, just let it through.
            chainCallback();
        });

    // Adding CORS headers to every response so the frontend can talk to the API.
    drogon::app().registerPostHandlingAdvice(
        [](const drogon::HttpRequestPtr &, const drogon::HttpResponsePtr &resp) {
            resp->addHeader("Access-Control-Allow-Origin", "*");
            resp->addHeader("Access-Control-Allow-Methods", "GET,POST,OPTIONS");
            resp->addHeader("Access-Control-Allow-Headers", "Content-Type");
        });

    drogon::app().run();
    return 0;
}
