#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <sass_interface.h>


typedef struct {
    ngx_flag_t  enable;
    ngx_uint_t  error_log;
    ngx_str_t   include_paths;
    ngx_uint_t  output_style;
    ngx_uint_t  precision;
    ngx_flag_t  source_comments;
} ngx_http_sass_loc_conf_t;


static ngx_int_t ngx_http_sass_init(ngx_conf_t *cf);
static void *ngx_http_sass_create_loc_conf(ngx_conf_t *cf);
static char *ngx_http_sass_merge_loc_conf(ngx_conf_t *cf,void *parent, void *child);
static char *ngx_http_sass_error_log_value(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static char *ngx_http_sass_output_value(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);


static ngx_command_t  ngx_http_sass_commands[] = {
    { ngx_string("sass_comments"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_FLAG,
      ngx_conf_set_flag_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_sass_loc_conf_t, source_comments),
      NULL },

    { ngx_string("sass_compile"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_FLAG,
      ngx_conf_set_flag_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_sass_loc_conf_t, enable),
      NULL },

    { ngx_string("sass_error_log"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_FLAG,
      ngx_http_sass_error_log_value,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_sass_loc_conf_t, error_log),
      NULL },

    { ngx_string("sass_include_paths"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_sass_loc_conf_t, include_paths),
      NULL },

    { ngx_string("sass_precision"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_num_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_sass_loc_conf_t, precision),
      NULL },

    { ngx_string("sass_output"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_http_sass_output_value,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_sass_loc_conf_t, output_style),
      NULL },

    ngx_null_command
};


static ngx_str_t  ngx_http_sass_type = ngx_string("text/css");


static ngx_str_t  err_levels[] = {
    ngx_null_string,
    ngx_string("emerg"),
    ngx_string("alert"),
    ngx_string("crit"),
    ngx_string("error"),
    ngx_string("warn"),
    ngx_string("notice"),
    ngx_string("info"),
    ngx_string("debug")
};


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
    size_t                     root;
    u_char                    *last;
    ngx_buf_t*                 b;
    ngx_chain_t                out;
    ngx_file_t                 file;
    ngx_str_t                  content, path;
    ngx_http_sass_loc_conf_t  *clcf;
    struct sass_file_context  *ctx;
    struct sass_options        options;

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

    ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "sass compile file: \"%V\"", &path);
    ngx_memzero(&file, sizeof(ngx_file_t));

    options.output_style    = clcf->output_style;
    options.precision       = (int) clcf->precision;
    options.source_comments = clcf->source_comments;
    options.include_paths   = (char *) clcf->include_paths.data;

    ctx             = sass_new_file_context();
    ctx->options    = options;
    ctx->input_path = (char *) path.data;

    sass_compile_file(ctx);

    if (ctx->error_status && ctx->error_message) {
        ngx_log_error(clcf->error_log, r->connection->log, 0, "sass compilation error: %s", ctx->error_message);
        sass_free_file_context(ctx);

        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    } else if (ctx->error_status) {
        ngx_log_error(clcf->error_log, r->connection->log, 0, "sass compilation error");
        sass_free_file_context(ctx);

        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));

    if (NULL == b) {
        ngx_log_error(clcf->error_log, r->connection->log, 0, "sass compilation error: %s", ctx->error_message);
        sass_free_file_context(ctx);

        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    out.buf  = b;
    out.next = NULL;

    content.len  = sizeof(ctx->output_string) - 1;
    content.data = ngx_pnalloc(r->pool, strlen(ctx->output_string));

    if (NULL == content.data) {
        ngx_log_error(clcf->error_log, r->connection->log, 0, "sass failed to allocate response buffer");
        sass_free_file_context(ctx);

        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    ngx_cpystrn(content.data, (unsigned char *) ctx->output_string, strlen(ctx->output_string));

    b->start    = b->pos = content.data;
    b->last     = b->end = content.data + strlen(ctx->output_string);
    b->memory   = 1;
    b->last_buf = 1;

    r->headers_out.status           = NGX_HTTP_OK;
    r->headers_out.content_type     = ngx_http_sass_type;
    r->headers_out.content_length_n = strlen(ctx->output_string);

    sass_free_file_context(ctx);
    ngx_http_send_header(r);

    return ngx_http_output_filter(r, &out);
}


static void *
ngx_http_sass_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_sass_loc_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_sass_loc_conf_t));

    if (NULL == conf) {
        return NULL;
    }

    conf->enable          = NGX_CONF_UNSET;
    conf->error_log       = NGX_LOG_ERR;
    conf->output_style    = SASS_STYLE_NESTED;
    conf->precision       = NGX_CONF_UNSET_UINT;
    conf->source_comments = NGX_CONF_UNSET;

    return conf;
}


static char *
ngx_http_sass_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_http_sass_loc_conf_t *prev = parent;
    ngx_http_sass_loc_conf_t *conf = child;

    ngx_conf_merge_off_value(conf->enable, prev->enable, 0);
    ngx_conf_merge_str_value(conf->include_paths, prev->include_paths, "");
    ngx_conf_merge_uint_value(conf->output_style, prev->output_style, SASS_STYLE_NESTED);
    ngx_conf_merge_uint_value(conf->precision, prev->precision, 5);
    ngx_conf_merge_off_value(conf->source_comments, prev->source_comments, 0);

    return NGX_CONF_OK;
}


static char *
ngx_http_sass_error_log_value(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_str_t  *value;
    ngx_uint_t  n;

    ngx_http_sass_loc_conf_t *slcf = conf;

    value = cf->args->elts;

    for (n = 1; n <= NGX_LOG_DEBUG; n++) {
        if (0 == ngx_strcmp(value[1].data, err_levels[n].data)) {
            slcf->error_log = n;
            return NGX_CONF_OK;
        }
    }

    ngx_conf_log_error(
        NGX_LOG_EMERG, cf, 0,
        "invalid sass_error_log parameter \"%V\"", &value[1]
    );

    return NGX_CONF_ERROR;
}


static char *
ngx_http_sass_output_value(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_str_t  *value;

    ngx_http_sass_loc_conf_t *slcf = conf;

    value = cf->args->elts;

    if (0 == ngx_strcmp(value[1].data, "nested")) {
        slcf->output_style = SASS_STYLE_NESTED;
        return NGX_CONF_OK;
    }

    if (0 == ngx_strcmp(value[1].data, "expanded")) {
        slcf->output_style = SASS_STYLE_EXPANDED;
        return NGX_CONF_OK;
    }

    if (0 == ngx_strcmp(value[1].data, "compact")) {
        slcf->output_style = SASS_STYLE_COMPACT;
        return NGX_CONF_OK;
    }

    if (0 == ngx_strcmp(value[1].data, "compressed")) {
        slcf->output_style = SASS_STYLE_COMPRESSED;
        return NGX_CONF_OK;
    }

    ngx_conf_log_error(
        NGX_LOG_EMERG, cf, 0,
        "invalid sass_output parameter \"%V\"", &value[1]
    );

    return NGX_CONF_ERROR;
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
