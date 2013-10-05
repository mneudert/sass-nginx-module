#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


typedef struct {
    ngx_flag_t   enable;
} ngx_http_sass_loc_conf_t;


static ngx_int_t ngx_http_sass_init(ngx_conf_t *cf);
static void *ngx_http_sass_create_loc_conf(ngx_conf_t *cf);
static char *ngx_http_sass_merge_loc_conf(ngx_conf_t *cf,void *parent, void *child);


static ngx_command_t  ngx_http_sass_commands[] = {
    { ngx_string("sass_compile"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_FLAG,
      ngx_conf_set_flag_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_sass_loc_conf_t, enable),
      NULL },

    ngx_null_command
};

static ngx_str_t  ngx_http_sass_type = ngx_string("text/css");

static ngx_http_module_t  ngx_http_sass_module_ctx = {
    NULL,                          /* preconfiguration */
    ngx_http_sass_init,            /* postconfiguration */

    NULL,                          /* create main configuration */
    NULL,                          /* init main configuration */

    NULL,                          /* create server configuration */
    NULL,                          /* merge server configuration */

    ngx_http_sass_create_loc_conf, /* create location configuration */
    ngx_http_sass_merge_loc_conf   /* merge location configuration */
};


ngx_module_t  ngx_http_sass_module = {
    NGX_MODULE_V1,
    &ngx_http_sass_module_ctx, /* module context */
    ngx_http_sass_commands,    /* module directives */
    NGX_HTTP_MODULE,           /* module type */

    NULL,                      /* init master */
    NULL,                      /* init module */
    NULL,                      /* init process */
    NULL,                      /* init thread */

    NULL,                      /* exit thread */
    NULL,                      /* exit process */
    NULL,                      /* exit master */
    NGX_MODULE_V1_PADDING
};


static ngx_int_t
ngx_http_sass_handler(ngx_http_request_t *r)
{
    size_t                     size, root;
    ssize_t                    n;
    u_char                    *scss, *last;
    ngx_http_complex_value_t   cv;
    ngx_file_t                 file;
    ngx_file_info_t            fi;
    ngx_str_t                  path;
    ngx_http_sass_loc_conf_t  *clcf;

    if (!(r->method & (NGX_HTTP_GET|NGX_HTTP_HEAD))) {
        return NGX_DECLINED;
    }

    clcf = ngx_http_get_module_loc_conf(r, ngx_http_sass_module);

    if (!clcf->enable) {
        return NGX_DECLINED;
    }

    last = ngx_http_map_uri_to_path(r, &path, &root, 0);

    if (NULL == last) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    path.len = last - path.data;

    ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "sass open file: \"%V\"", &path);
    ngx_memzero(&file, sizeof(ngx_file_t));

    file.name = path;
    file.log  = r->connection->log;
    file.fd = ngx_open_file(path.data, NGX_FILE_RDONLY, 0, 0);

    if (NGX_INVALID_FILE == file.fd) {
        ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "sass open file error");
        return NGX_DECLINED;
    }

    if (NGX_FILE_ERROR == ngx_fd_info(file.fd, &fi)) {
        ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "sass fd info error");
        goto declined;
    }

    size = (size_t) ngx_file_size(&fi);
    scss = ngx_palloc(r->pool, size);

    if (NULL == scss) {
        ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "sass palloc error");
        goto declined;
    }

    n = ngx_read_file(&file, scss, size, 0);

    if (NGX_ERROR == n) {
        ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "sass read file error");
        goto declined;
    }

    if ((size_t) n != size) {
        ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "sass read file wrong size");
        goto declined;
    }


    ngx_memzero(&cv, sizeof(ngx_http_complex_value_t));

    cv.value.len = size;
    cv.value.data = scss;

    if (NGX_FILE_ERROR == ngx_close_file(file.fd)) {
        ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "sass close file error");
    }

    return ngx_http_send_response(r, NGX_HTTP_OK, &ngx_http_sass_type, &cv);

declined:

    if (NGX_FILE_ERROR == ngx_close_file(file.fd)) {
        ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "sass close file error");
    }

    return NGX_DECLINED;
}


static void *
ngx_http_sass_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_sass_loc_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_sass_loc_conf_t));

    if (NULL == conf) {
        return NULL;
    }

    conf->enable = NGX_CONF_UNSET;

    return conf;
}


static char *
ngx_http_sass_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_http_sass_loc_conf_t *prev = parent;
    ngx_http_sass_loc_conf_t *conf = child;

    ngx_conf_merge_value(conf->enable, prev->enable, 0);

    return NGX_CONF_OK;
}


static ngx_int_t
ngx_http_sass_init(ngx_conf_t *cf)
{
    ngx_http_handler_pt       *h;
    ngx_http_core_main_conf_t *cmcf;

    cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);
    h    = ngx_array_push(&cmcf->phases[NGX_HTTP_CONTENT_PHASE].handlers);

    if (NULL == h) {
        return NGX_ERROR;
    }

    *h = ngx_http_sass_handler;

    return NGX_OK;
}
