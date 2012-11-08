
/**
 * @file
 * The primary AHAH behaviors and handlers for AJAX commenting.
 */

var commentbox = ".comment";
var ctrl = false;
var last_submit;
var speed = 'fast';
var ahah = false;
var firsttime_init = true;

/**
 * Attaches the ahah behavior to each ahah form element.
 */
Drupal.behaviors.ajax_comments = function(context) {
  Drupal.ajax_comments_init_form(context);
  Drupal.ajax_comments_init_links(context);
  if (Drupal.settings.comment_bonus_api_fold_comments) {
    Drupal.ajax_comments_fold(context);
  }

  // Add Ctrl key listener for deletion feature.
  $(window).keydown(function(e) {
    if(e.keyCode == 17) {
      ctrl = true;
    }
  });
  $(window).keyup(function(e) {
    ctrl = false;
     // Add sending on Ctrl+Enter.
    if ((e.ctrlKey) && ((e.keyCode == 0xA) || (e.keyCode == 0xD)) && ((typeof submitted == "undefined") || !submitted)) {
      submitted = true;
      var form_num = ajax_comments_get_form_num_from_element(e.target);
      $('#ajax-comments-submit-' + form_num).click();
    }
 });

  firsttime_init = false;
};

/**
 * Attach behaviors to comment form.
 *
 * This function works with the comment entry form on the page. It binds a few different events
 * to enable further actions to take place. It also adds the <h2> title link, which is used later
 * as an on-page placement guide for a number of other actions (previews, new comments, etc.)
 */
Drupal.ajax_comments_init_form = function(context) {
  $('.comment-form:not(.ajax-comments-processed)', context).addClass('ajax-comments-processed').each(function() {
    form = $(this);   
    
    // Extract form number from id
    var form_num = form.attr("id").substr(form.attr("id").search(/-(\d)$/) + 1);
    var nid = ajax_comments_get_nid_from_href(form.attr('action'));
    
    // CSS selectors for submit and preview buttons 
    var comments_submit_sel = '#ajax-comments-submit-'  + form_num;
    var preview_submit_sel  = '#ajax-comments-preview-' + form_num;
    var comments_n_preview_sel = comments_submit_sel + ',' + preview_submit_sel;
    
    // Prepare the form when the DOM is ready.
    if ((Drupal.settings.ajax_comments_rows_default == undefined) || (!Drupal.settings.ajax_comments_rows_default)) {
      Drupal.settings.ajax_comments_rows_default = $('textarea', form).attr('rows');
    }
    $('textarea', form).attr('rows', Drupal.settings.ajax_comments_rows_default);
    if ((Drupal.settings.ajax_comments_rows_in_reply == undefined) || (!Drupal.settings.ajax_comments_rows_in_reply)) {
      Drupal.settings.ajax_comments_rows_in_reply = Drupal.settings.ajax_comments_rows_default;
    }
    if (Drupal.settings.ajax_comments_always_expand_form == undefined) {
      Drupal.settings.ajax_comments_always_expand_form = true;
    }
    if (Drupal.settings.ajax_comments_blink_new == undefined) {
      Drupal.settings.ajax_comments_blink_new = true;
    }

    $('#edit-upload-' + form_num, form).bind('change', function(){
      $(comments_n_preview_sel, form).attr('disabled', 1);
    });
    
    // It's not possible to use 'click' or 'submit' events for ahah sumits, so
    // we should emulate it by up-down events. We need to check which elements
    // are actually clicked pressed, to make everything work correct.
           
    $(comments_n_preview_sel, form).bind('mousedown keydown', function() {
    	last_submit = $(this).attr('id');    	    	
    });
    $(comments_n_preview_sel, form).bind('mouseup', function() {
      if (last_submit == $(this).attr('id')) {
        ajax_comments_show_progress(form_num, context);
        ajax_comments_update_editors();
      }
    });
    $(comments_n_preview_sel, form).bind('keyup', function(event) {
      if (last_submit == $(this).attr('id') && event.keyCode == 13) {
        ajax_comments_show_progress(form_num, context);
        ajax_comments_update_editors();
      }
    });
    
    // Enable comments buttons back when attachement is uploaded.
    $('#edit-attach-' + form_num, form).bind('mousedown keydown', function() {
      if (last_submit == $(this).attr('id')) {
        $(comments_n_preview_sel, form).removeAttr('disabled');
      }
    });

    // Initializing main form.
    action = form.attr('action');
    
    // Creating title link.
    form.parent('div').prev('h2:not(.ajax-comments-title-processed),h3:not(.ajax-comments-title-processed),h4:not(.ajax-comments-title-processed)').addClass('ajax-comments-title-processed').each(function(){
      title = $(this).html();
      $(this).html('<a href="'+action+'" id="comment-form-title-' + form_num + '">'+title+'</a>');
      form.parent('div').attr('id','comment-form-content-' + form_num).removeClass("content");
    });

    // Expanding form if needed.
    page_url = document.location.toString();
    fragment = '';
    if (page_url.match('#')) {
      fragment = page_url.split('#')[1];
    }
    
    var title_sel   = '#comment-form-title-'   + form_num;
    var content_sel = '#comment-form-content-' + form_num; 

    if ((fragment == 'comment-form' + form_num  || Drupal.settings.ajax_comments_always_expand_form) && firsttime_init) {
      $(title_sel, context).addClass('pressed');
      $(content_sel).attr('cid', 0);
    }
    else {
      // Fast hide form.
      $(content_sel, context).hide();
    }
    
    // Attaching event to title link.
    $(title_sel + ':not(.ajax-comments-processed)', context).addClass('ajax-comments-processed').click(Drupal.ajax_comments_reply_click);

    // Moving preview in a proper place.
    if (ajax_comments_is_reply_to_node(action)) {
      $('#node-' + nid).siblings('div#comments').find('.ajax-comments-title-processed').before($('#comment-preview-' + form_num));
    }
    else {
      $(content_sel).before($('#comment-preview-' + form_num));
    }

    if (!$(content_sel).attr('cid')) {
      $(content_sel).attr('cid', -1);
    }
    
    if(typeof(fix_control_size)!='undefined'){ fix_control_size(); }
    
  });
}

/**
 * Attach behaviors to comment links.
 * 
 * This function adds event handlers for when a user clicks on certain page elements, including
 * reply links, quote links, and edit links. 
 */
Drupal.ajax_comments_init_links = function(context) {
  // Process reply links.
  $('.comment_reply a:not(.ajax-comments-processed)', context).addClass('ajax-comments-processed').click(Drupal.ajax_comments_reply_click);

  // Process quote links.
  $('.quote a:not(.ajax-comments-processed)', context).addClass('ajax-comments-processed').each(function(){
    href = $(this).attr('href');
    if (ajax_comments_is_reply_to_node(href)) {
      var form_num = ajax_comments_get_form_num_from_href(href);
      
      $(this).click(function(){
        $('#comment-form-' + form_num).attr('action', $(this).attr('href'));
        ajax_comments_reload_form(form_num, 0, 'pid');

        $('#comment-form-title-' + form_num, context).click();
        ajax_comments_scroll_to_comment_form(form_num);
        return false;
      });
    }
    else {
      $(this).click(Drupal.ajax_comments_reply_click);
    }
  });

  // Process edit links.
  $('.comment_edit a:not(.ajax-comments-processed)', context).addClass('ajax-comments-processed').click(Drupal.ajax_comments_edit_click);

  // We should only bind ajax deletion on links with tokens to avoid CSRF attacks.
  $('.comment_delete a:not(.ajax-comments-processed)', context).addClass('ajax-comments-processed').each(function (){
    href = $(this).attr('href');
    if (href.indexOf('token=') > -1) {
      $(this).addClass('ajax-comments-processed').click(Drupal.ajax_comments_delete_click);
    }
  });
}

/**
 * Fold indednted comments threads.
 * 
 * TODO: check div selector
 */
Drupal.ajax_comments_fold = function(context) {
  $('div[id^=comments-] > .indented:not(.ajax-comments-processed)', context).addClass('folded').addClass('ajax-comments-processed').each(function (){
    $thread = $(this);
    // Hide threads.
    $thread.css('display', 'none');

    // Calculate child elements.
    num_replies = $thread.children(commentbox).size();

    // Find placeholder for thread folding links.
    $fold_links = $thread.prev(commentbox).find('.fold-links');
    if (!$fold_links.length) {
      $thread.prev(commentbox).append('<div class="fold-links"></div>');
      $fold_links = $thread.prev(commentbox).find('.fold-links');
    }
    // Draw control elements.
    $fold_links.html('<ul class="links"><li class="num-replies"><span class="num-replies-inner">' + Drupal.formatPlural(num_replies, '1 Reply', '@count Replies') + '</span></li><li class="toggle-thread"><a href="#" class="show-thread">' + Drupal.t('Show') + '</a></li></ul>');
  });

  $('.fold-links > ul > .toggle-thread a:not(.ajax-comments-processed)', context).addClass('ajax-comments-processed').click(function (){
    $toggler = $(this);
    if ($toggler.is('.show-thread')) {
      $toggler.parents(commentbox).next('.indented').slideDown();
      $toggler.text(Drupal.t('Hide'))
        .removeClass('show-thread')
        .addClass('hide-thread');
    }
    else if ($toggler.is('.hide-thread')) {
      $toggler.parents(commentbox).next('.indented').slideUp();
      $toggler.text(Drupal.t('Show'))
        .removeClass('hide-thread')
        .addClass('show-thread');
    }
    return false;
  });
}


/**
 * Reply links handler.
 * 
 * This function is called as the handler to the .click() event. The events were bound
 * during the init functions above. This is the function executed when the user clicks
 * on a 'reply' button on the page or the 'Post new comment' link.
 */
Drupal.ajax_comments_reply_click = function() {
	
  // We should only handle non presed links.
  if (!$(this).is('.pressed')){
    action = $(this).attr('href');
    var comment_nid = ajax_comments_get_nid_from_href(action);
    var form_num = ajax_comments_get_form_num_from_href(action);

    form_action = $('#comment-form-' + form_num).attr('action');
    link_cid = ajax_comments_get_cid_from_href(action);
    rows = Drupal.settings.ajax_comments_rows_default;
    
    // CSS selectors
    var comment_content_sel = '#comment-form-content-' + form_num;
    
    if ($('#comment-form-content-' + form_num).attr('cid') != link_cid) {
      // We should remove any WYSIWYG before moving controls.
      ajax_comments_remove_editors();

      // Move form from old position.
      if (ajax_comments_is_reply_to_node(action)) {
        $(comment_content_sel).removeClass('indented');
        if ($(comment_content_sel + ':visible').length) {
          $(comment_content_sel).after('<div style="height:' + $(comment_content_sel).height() + 'px;" class="sizer"></div>');
          $('.sizer').slideUp(speed, function(){ $(this).remove(); });
        }
        $(this).parents('h2,h3,h4').after($(comment_content_sel));
        rows = Drupal.settings.ajax_comments_rows_default;
        $('.ajax-comments-title-processed').before($('#comment-preview-' + form_num));
      }
      else {
        $(comment_content_sel).addClass('indented');
        if ($(comment_content_sel + ':visible').length) {
          $(comment_content_sel).after('<div style="height:' + $(comment_content_sel).height() + 'px;" class="sizer"></div>');
          $('.sizer').slideUp(speed, function(){ $(this).remove(); });
        }
        
        folded_thread = $(this).parents(commentbox).next('.indented.folded');
        if (folded_thread.length && Drupal.settings.comment_bonus_api_fold_comments) {
          $(this).parents(commentbox).find('.hide-thread').click();
          folded_thread.after($(comment_content_sel));
        }
        else {
          $(this).parents(commentbox).after($(comment_content_sel));
        }
        rows = Drupal.settings.ajax_comments_rows_in_reply;
        $(comment_content_sel).prepend($('#comment-preview-' + form_num));
      }
      $(comment_content_sel).hide();
    }

    // We don't need to load everything twice.
    if (!$(this).is('.last-clicked')) {
      // Reload form if preview is required.
      if ((Drupal.settings.comment_preview_required && $('#ajax-comments-submit-' + form_num).length) ||
        // Or if quoted comment or custom reload trigger.
        action.match('quote=1') || form_action.match('reload=1')
      ) {
        $('#comment-form-' + form_num).attr('action', action)
        ajax_comments_reload_form(form_num, link_cid, 'pid');
        
        $('.editing').fadeTo('fast', 1);
      }
      else {
        ajax_comments_rewind(form_num, link_cid, rows);
        ajax_comments_scroll_to_comment_form(form_num);
      }
    }
    // ...and show the form after everything is done.
    ajax_comments_expand_form(form_num);
    
    $('.pressed').removeClass('pressed');
    $(this).addClass('pressed');
    $('.last-clicked').removeClass('last-clicked');
    $(this).addClass('last-clicked');
    $(comment_content_sel).attr('cid', link_cid);
  }
  else {
    // Handling double click.
    if ((!$(this).is('#comment-form-title-' + form_num)) && (Drupal.settings.ajax_comments_always_expand_form)) {
      $('#comment-form-title-' + form_num).click();
    }
    else if (!Drupal.settings.ajax_comments_always_expand_form) {
      ajax_comments_close_form(form_num);
    }
  }

  if (typeof(fix_control_size) != 'undefined'){ fix_control_size(); }
  return false;
}

/**
 * Edit links handler.
 *
 * This function is called as the handler to the .click() event. This event was bound
 * during the init function above. This is the function executed when the user clicks
 * on an 'edit' button on the page.
 */
Drupal.ajax_comments_edit_click = function() {
  $edit_link = $(this);
  var form_num = ajax_comments_get_form_num_from_element(this);
  
  // CSS selectors
  var comment_content_sel = '#comment-form-content-' + form_num;  

  $edit_link.parents(commentbox).fadeTo('fast', 0.5).addClass('editing');

  ajax_comments_show_progress(form_num);
  $(comment_content_sel).addClass('indented');
  if ($(comment_content_sel + ':visible').length) {
    $(comment_content_sel).after('<div style="height:' + $(comment_content_sel).height() + 'px;" class="sizer"></div>');
    $('.sizer').slideUp(speed, function(){ $(this).remove(); });
  }
  
  $edit_link.parents(commentbox).after($(comment_content_sel));
  rows = Drupal.settings.ajax_comments_rows_in_reply;
  $(comment_content_sel).prepend($('#comment-preview-' + form_num));
  $(comment_content_sel).hide();
  ajax_comments_expand_form(form_num);

  form_action = $('#comment-form-' + form_num).attr('action');
  // Reload form with edit data.
  $args = ajax_comments_get_args(form_action);
  nid = $args[2];
  action = $edit_link.attr('href');
  action = action.replace('comment/edit', 'ajax_comments/js_reload/' + nid) + '/cid';
  ajax_comments_reload_form(form_num, action, 'action', function() { 
    // Set reload trigger.	  
    $('#comment-form-' + form_num).attr('action', form_action + '?reload=1');
    $(comment_content_sel).attr('cid', 'edit');
  });
  
  $('.pressed').removeClass('pressed');
  $(this).addClass('pressed');  
  $('.last-clicked').removeClass('last-clicked');
  $(this).addClass('last-clicked');
    
  return false;
}

/**
 * Delete links handler.
 * 
 * This function is called as the handler to the .click() event. The event was bound
 * during the init function above. This is the function executed when the user clicks
 * on a 'delete' button on the page.
 */
Drupal.ajax_comments_delete_click = function() {
  if ((ctrl) || (confirm(Drupal.t('Are you sure you want to delete the comment? Any replies to this comment will be lost. This action cannot be undone.')))) {
    var form_num = ajax_comments_get_form_num_from_element(this);
    // Taking link's href as AJAX url.
    comment = $(this).parents(commentbox);
    action = $(this).attr('href');
    action = action.replace(/comment\/delete\//, 'ajax_comments/instant_delete/');
    if (action) {
      $(this).parents(commentbox).fadeTo(speed, 0.5);
      $.ajax({
        type: "GET",
        url: action,
        success: function(response) {
          if (response.status) {
            ajax_comments_close_form(form_num);

            // If comment form is expanded on this module, we should collapse it first.
            if (comment.next().is('#comment-form-content-' + form_num)) {
              thread = comment.next().next('.indented, div > .indented');
            }
            else {
              thread = comment.next('.indented, div > .indented');
            }
            thread.animate({height:'hide', opacity:'hide'}, speed);
            comment.animate({height:'hide', opacity:'hide'}, speed, function(){
              thread.remove();
              comment.remove();
              if (!$(commentbox).length) {
                $('#comment-controls-' + form_num).animate({height:'hide', opacity:'hide'}, speed, function(){ $(this).remove(); });
              }
            });
          }
          else {
            alert('Sorry, token error.');
          }
        },
        dataType: 'json'
      });
    }
  }
  return false;
}

/**
 * Attaches the ahah behavior to each ahah form element.
 */
Drupal.behaviors.ajax_comments_pager = function(context) {
  $('div[id^=comments-] .pager:not(.pager-processed)', context).addClass('pager-processed').each(function() {
    $target = $(this);
    $target
      .find('li > a')
      .click(function () {
        $(this).animate({paddingRight:16}, 'fast').addClass('throbber').removeAttr('style');
        Drupal.turn_over_page($target, $(this).attr('href'), true, function(){}, function(){ $(this).removeClass('throbber'); });
        return false;
      });
  });
}

/**
 * Turn over a single page.
 *
 * This function is called as the handler to the .click() event. The event was bound
 * during the Drupal.behaviors.ajax_comments_pager function above. This is the function
 * executed when the user clicks on one of the links in the pager for a specific page
 * or the next/last/first links.
 *
 *   @param target
 *     The .pager element.
 *   @param url
 *     New page URL.
 *   @param scroll
 *     Is scroll to comments header needed.
 *   @param success_callback
 *     Function which will be called after pagination (or immediately if pager does not exists).
 *   @param error_callback
 *     Function which will be called on AJAX error.
 */
Drupal.turn_over_page = function(target, url, scroll, success_callback, error_callback) {
  if (target.length && url) {
    ajaxPath = url.replace(/(.*?)\?(.*?)/g, Drupal.settings.basePath + '/ajax_comments/js_load_thread/' + Drupal.settings.ajax_comments_nid + '?$2');
    $.ajax({
      url: ajaxPath,
      type: 'GET',
      success: function(response) {
        if (response.status && response.content) {
          if (scroll) {
            offset = $('#comments').offset();
            $('html').animate({scrollTop: offset.top}, 'slow');
          }
          target.parent().parent().animate({opacity: 0.2}, 'fast', function() { 
            var $newContent = $(response.content);
            target.parent().parent().replaceWith($newContent);
            firsttime_init = true;
            Drupal.attachBehaviors($newContent.parent());

            if (scroll) {
              offset = $('#comments').offset();
              $('html').animate({scrollTop: offset.top}, success_callback);
            }
            else {
              success_callback();
            }
          });
        }
      },
      error: function() {
        error_callback();
        alert(Drupal.t("An error occurred at @path.", {'@path': ajaxPath}));
      },
      dataType: 'json'
    });
  }
  else {
    success_callback();
  }
}

// ======================================================================
// Misc. functions
// ======================================================================

/**
 * Hide comment form, reload if needed.
 */
function ajax_comments_expand_form(form_num, focus) {
  $('#comment-form-content-' + form_num).animate({height:'show'}, speed, function() {
    if (focus) {
      $('#comment-form-' + form_num + ' textarea').focus();
    }
    if ($.browser.msie) this.style.removeAttribute('filter'); 
  });
}

/**
 * Helper function for reply handler.
 */
function ajax_comments_rewind(form_num, pid, rows){
  // Resizing and clearing textarea.
  $('#comment-form-' + form_num + ' textarea').attr('rows', rows);
  $('#comment-form-' + form_num + ':not(.fresh) textarea').attr('value','');

  // Clearing form.
  $('#comment-preview-' + form_num).empty();
  $('#comment-form .error').removeClass('error');

  // Set proper PID.
  $('#comment-form-' + form_num + ' input[name=pid]').val(pid)

  // Now we can attach previously removed editors.
  ajax_comments_attach_editors();
  submit = false;
}

/**
 * Hide comment form, reload if needed.
 */
function ajax_comments_close_form(form_num, reload) {
  pid = $('#comment-form-content-' + form_num).attr('cid');
  $('#comment-form-content-' + form_num).animate({height:'hide'}, speed, function(){
    if (reload) {
      ajax_comments_reload_form(form_num, pid, 'pid');
    }
  });
  $('.pressed').removeClass('pressed');
  $('#comment-form-content-' + form_num).attr('cid', -1);
  ajax_comments_hide_progress(form_num);
}

/**
 * Reload comments form from server.
 */
function ajax_comments_reload_form(form_num, id, type, callback) {	
  rows = Drupal.settings.ajax_comments_rows_default;
  if (type == 'pid') {
    action = $('#comment-form-' + form_num).attr('action');
    action = action.replace(/comment\/reply\/([0-9]+?)(\/*[0-9]*)$/, 'ajax_comments/js_reload/$1$2');
    action = action.replace(/ajax_comments\/js_reload\/([0-9]+?)\/([0-9]+?)\/cid/, 'ajax_comments/js_reload/$1');

    if (id > 0) {
      action = action.replace(/([?])$/, '/' + id + '?');
      action = action.replace('/#comment-form-' + form_num + '/', '');
      
      rows = Drupal.settings.ajax_comments_rows_in_reply;
    }
  }
  else if (type == 'action') {
    action = id;
  }

  $('#comment-preview-' + form_num).hide();
  ajax_comments_show_progress(form_num);

  $.ajax({
    type: "GET",
    url: action,
    success: function(response) {
     if (response.status && response.content) {
        saved_class = $('#comment-form-' + form_num).attr('class');
        saved_class = saved_class.replace('ajax-comments-processed', '');
        content = response.content;
        // form_clean_id() will not work with JSON request so we need to extend 
        // form id and other form elements with previous unique number
        if (form_num > 1) {
          content = content.replace(/comment-preview-1/gm, 'comment-preview-' + form_num);
          content = content.replace(/comment-form-1/gm,    'comment-form-' + form_num);
          content = content.replace(/ajax-comments-submit-1/gm,  'ajax-comments-submit-'  + form_num);
          content = content.replace(/ajax-comments-preview-1/gm, 'ajax-comments-preview-' + form_num);          
        }

        $('#comment-form-content-' + form_num).html(content);
        $('#comment-form-' + form_num).attr('class', saved_class);

        $('#comment-form-' + form_num).addClass('fresh');
        Drupal.attachBehaviors($('#comment-form-content-' + form_num));
        ajax_comments_rewind(form_num, id, rows);
        ajax_comments_hide_progress(form_num);

        $('#comment-form-' + form_num).removeClass('fresh');

        if (undefined != callback) {
          callback();
        }
      }     
    },
    dataType: 'json'
  });
}

/**
 * Scrolling to a new comment.
 */
function ajax_comments_scroll_to_comment_form(form_num) {
  if ($.browser.msie) {
    height = document.documentElement.offsetHeight ;
  }
  else if (window.innerWidth && window.innerHeight) {
    height = window.innerHeight;
  }
  height = height / 2;
  offset = $('#comment-form-content-' + form_num).offset();
  if ((offset.top > $('html').scrollTop() + height) || (offset.top < $('html').scrollTop() - 20)) {
    $('html').animate({scrollTop: offset.top}, 'slow');
  }
}

/**
 * Find a place for a new comment.
 */
function ajax_comments_insert_new_comment(form_num, nid, $comment) {
  var form_content_sel = '#comment-form-content-' + form_num;
    
  if ($(form_content_sel).attr('cid') == 'edit') {
    $(form_content_sel).before($comment);
    $('.editing').remove();
    $(form_content_sel).attr('cid', 0);
  }
  else if ($(form_content_sel).attr('cid') <= 0) {
    if ($('#comments-' + nid + ' .pager').length) {
      $('#comment-preview-' + form_num).prev('.item-list').before($comment);
    }
    else {
      $('#comment-preview-' + form_num).before($comment);
    }
  }
  else {
    if ($(form_content_sel).next().is('.indented')) {
      $(form_content_sel).next().append($comment);
    }
    else {
      $(form_content_sel).before($comment);
      $comment.wrap('<div class="indented"></div>');
    }
  }
}

/**
 * AHAH effect for comment previews.
 * This effect was bound via ajax_comments.module in ajax_comments_form_comment_form_alter()
 * as the AHAH effect.
 *
 * @see http://drupal.org/node/331941
 */
jQuery.fn.ajaxCommentsPreviewToggle = function() {
  var obj = $(this[0]);

  //var form_num = ajax_comments_get_form_num_from_element(this[0]);
  var comments_form = obj.parents('div[id^=comments-]');
  alert(obj.parent().html());
  
  // Hide previous preview.
  $('#comment-preview-' + form_num + ' > div:visible').animate({height:'hide', opacity:'hide'}, speed, function() { $(this).remove(); } );
  // Show fresh preview.
  $('#comment-preview-' + form_num).show();
  obj.animate({height:'show', opacity:'show'}, speed);
  ajax_comments_hide_progress(form_num);

  // Add submit button if it doesn't added yet.
  if (!$('#ajax-comments-submit-' + form_num).length && $('.preview-item').length) {
    $('#ajax-comments-preview-' + form_num).after('<input name="op" id="ajax-comments-submit-' + form_num + '" value="'+ Drupal.t("Save") +'" class="form-submit" type="submit">');
    // Re-attaching to new comment.
    Drupal.attachBehaviors($('#ajax-comments-submit-' + form_num));
  }
};

/**
 * AHAH effect for comment submits.
 */
jQuery.fn.ajaxCommentsSubmitToggle = function() {	
  var obj = $(this[0]);      
  var html = obj.html();
  var link = obj.find('a[href*=/comment/reply]');
  var form_num = ajax_comments_get_form_num_from_href(link.attr('href'));
  var nid = ajax_comments_get_nid_from_href(link.attr('href'));
  
  if (html.indexOf('comment-new-success') > -1) {
    // Empty any preview before output comment.
    $('#comment-preview-' + form_num).slideUp(speed, function(){ $(this).empty(); });

    // If there are more than one page in comments tthread, we should firstly turn over to a last page.
    $last_page = $('#comments-' + nid + ' .pager:first li.pager-last a');
    if (!$last_page.length) {
      $last_page = $('#comments-' + nid + ' .pager:first li.pager-next').prev().children('a');
    }
    if ($('#comment-form-content-' + form_num).attr('cid') <= 0 && $last_page.length) {
      Drupal.turn_over_page($('#comments-' + nid + ' .pager:first'), $last_page.attr('href'), false, function(){}, function(){ $(this).removeClass('throbber'); });
    }
    else {
      // Place new comment in proper place.
      ajax_comments_insert_new_comment(form_num, nid, obj);

      offset = obj.offset();
      $('html').animate({scrollTop: offset.top});
      // At last - showing it up. 
      obj.animate({height:'show', opacity:'show'}, speed, function () {
        if ($.browser.msie) {
          height = document.documentElement.offsetHeight ;
        } else if (window.innerWidth && window.innerHeight) {
          height = window.innerHeight;
        }
        height = height / 2;
        offset = obj.offset();
        if ((offset.top > $('html').scrollTop() + height) || (offset.top < $('html').scrollTop() - 20)) {
          $('html').animate({scrollTop: offset.top - height}, 'slow', function(){
            // Blink a little bit to user, so he know where's his comment.
            if (Drupal.settings.ajax_comments_blink_new) {
              obj.fadeTo('fast', 0.2).fadeTo('fast', 1).fadeTo('fast', 0.5).fadeTo('fast', 1).fadeTo('fast', 0.7).fadeTo('fast', 1, function() { if ($.browser.msie) this.style.removeAttribute('filter'); });
            }
          });
        }
        else {
          if (Drupal.settings.ajax_comments_blink_new) {
            obj.fadeTo('fast', 0.2).fadeTo('fast', 1).fadeTo('fast', 0.5).fadeTo('fast', 1).fadeTo('fast', 0.7).fadeTo('fast', 1, function() { if ($.browser.msie) this.style.removeAttribute('filter'); });
          }
        }
        if ($.browser.msie) this.style.removeAttribute('filter');
      });

      // Re-attaching behaviors to new comment.
      Drupal.attachBehaviors(html);

      // Hiding comment form.
      ajax_comments_close_form(form_num, true);

      if (Drupal.settings.ajax_comments_always_expand_form) {
        $('#comment-form-title-' + form_num).click();
      }
    }
  }
  else {
    $('#comment-preview-' + form_num).append(obj);
    obj.ajaxCommentsPreviewToggle(speed);
  }
};

/**
 * Remove editors from comments textarea (mostly to re-attach it).
 */
function ajax_comments_remove_editors() {
  ajax_comments_update_editors();
  if (typeof(Drupal.wysiwyg) != undefined) {
    $('#comment-form input.wysiwyg-processed:checked').each(function() {
      var params = Drupal.wysiwyg.getParams(this);
      Drupal.wysiwygDetach($(this), params);
    });
    return;
  }
  
  if (typeof(tinyMCE) != 'undefined') {
    if (tinyMCE.getInstanceById("edit-comment")) {
      tinyMCE.execCommand('mceRemoveControl', false, "edit-comment");
    }
  }
}

/**
 * Attach editors to comments textarea if needed.
 */
function ajax_comments_attach_editors() {
  if (typeof(Drupal.wysiwyg) != undefined) {
    $('#comment-form input.wysiwyg-processed:checked').each(function() {
      var params = Drupal.wysiwyg.getParams(this);
      Drupal.wysiwygAttach($(this), params);
    });
    return;
  }

  if (typeof(tinyMCE) != 'undefined') {
    tinyMCE.execCommand('mceAddControl', false, "edit-comment");
  }
}

/**
 * Update editors text to their textareas. Need to be done befor submits.
 */
function ajax_comments_update_editors() {
  // Update tinyMCE.
  if (typeof(tinyMCE) != 'undefined') {
    tinyMCE.triggerSave();
  }
  
  // Update FCKeditor.
  if (typeof(doFCKeditorSave) != 'undefined') {
    doFCKeditorSave();
  }
  if(typeof(FCKeditor_OnAfterLinkedFieldUpdate) != 'undefined'){
    FCKeditor_OnAfterLinkedFieldUpdate(FCKeditorAPI.GetInstance('edit-comment'));
  }
}

function ajax_comments_get_cid_from_href(action) {
  args = ajax_comments_get_args(action);

  // Getting token params (/comment/delete/!cid!).
  if (args[1] == 'delete') {
    cid = args[2];
  }
  // Getting token params (/comment/reply/nid/!cid!).
  else {
    if (typeof(args[3]) == 'undefined') {
      cid = 0;
    }
    else {
      cid = args[3];
    }
  }
  return cid;
}

function ajax_comments_get_nid_from_href(action) {
	args = ajax_comments_get_args(action);

	// Getting token params (/comment/reply/nid/!cid!).
	if (args[1] == 'reply') {
		if (typeof(args[2]) == 'undefined') {
			nid = 0;
		} else {
			nid = args[2];
		}
	} else {
		nid = 0;
	}
	return nid;
}

/**
 * Retrieve related comment form number from reply link
 */
function ajax_comments_get_form_num_from_href(href) {	
	var nid = ajax_comments_get_nid_from_href(href);
    var form_id = $('#node-' + nid).siblings('div#comments').find('form.comment-form').attr('id');
    return form_id.substr(form_id.search(/-(\d)$/) + 1);	
}

function ajax_comments_get_form_num_from_element(el) {
	var comments_form = $(el).parents('div#comments');
    var form_id = comments_form.find('form.comment-form').attr('id');
    return form_id.substr(form_id.search(/-(\d)$/) + 1);	
}

function ajax_comments_is_reply_to_node(href) {
  args = ajax_comments_get_args(href);
  result = args[1] == 'reply' && args[2] && (typeof(args[3]) == 'undefined');
  return result;
}

function ajax_comments_get_args(url) {
  //if (Drupal.settings.clean_url[0] == '1') {
    var regexS = "(http(s)*:\/\/)*([^/]*)"+ Drupal.settings.basePath +"([^?#]*)";
    var regex = new RegExp( regexS );
    var results = regex.exec( url );
    args = results[4];
  //}
  /*else {
    var regexS = "([&?])q=([^#&?]*)";
    var regex = new RegExp( regexS );
    var results = regex.exec( url );
    args = results[2];
  }*/
  args = args.split('/');
  if (Drupal.settings.language_mode == 1 || Drupal.settings.language_mode == 2) {
    for (l in Drupal.settings.language_list) {
      if (args[0] == Drupal.settings.language_list[l].language) {
        args.shift();
        break;
      }
    }
  }
  return args;
}

function ajax_comments_show_progress(form_num, context) {
  if (!context) {
    context = '#comment-form-content-' + form_num;
  }
  if (!$('#comment-form-' + form_num + ' .ajax-comments-loader', context).length) {
    $('#comment-form-' + form_num, context).append('<div class="ajax-comments-loader"></div>');
  }
}

function ajax_comments_hide_progress(form_num, context) {
  if (!context) {
    context = '#comment-form-content-' + form_num;
  }
  $('#comment-form-' + form_num + ' .ajax-comments-loader', context).fadeOut(speed, function(){ $(this).remove(); });
}