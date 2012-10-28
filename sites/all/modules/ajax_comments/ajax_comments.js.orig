
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
 * Attach AHAH behavior to each AHAH form element.
 *
 * Define a function as a property of Drupal.behaviors. This will call the code
 * block when the DOM has finished loading. In this case, we attach the AHAH
 * behavior to each AHAH form element.
 *
 * @see http://drupal.org/node/205296
 */
Drupal.behaviors.ajax_comments = function(context) {
  Drupal.ajax_comments_init_form(context);
  Drupal.ajax_comments_init_links(context);
  if (Drupal.settings.comment_bonus_api_fold_comments) {
    Drupal.ajax_comments_fold(context);
  }

  // The CTRL key listener is for the comment deletion feature. 
	// It allows for bypassing the deletion confirmation dialog.
	$(window).keydown(function(e) {
    if(e.keyCode == 17) {
      ctrl = true;
    }
  });
  $(window).keyup(function(e) {
    ctrl = false;
     // Add sending on Ctrl+Enter.
    if ((e.ctrlKey) && ((e.keyCode == 0xA) || (e.keyCode == 0xD)) && !submitted) {
      submitted = true;
      $('#ajax-comments-submit').click()
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
  $('#comment-form:not(.ajax-comments-processed)', context).addClass('ajax-comments-processed').each(function() {
    form = $(this);
    
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

    $('#edit-upload', form).bind('change', function(){
      $('#ajax-comments-submit,#ajax-comments-preview', form).attr('disabled', 1);
    });
    
    // It's not possible to use 'click' or 'submit' events for AHAH sumits, so
    // we should emulate it by up-down events. We need to check which elements
    // are actually clicked pressed, to make everything work correct.
    $('#ajax-comments-submit,#ajax-comments-preview', form).bind('mousedown keydown', function() { last_submit = $(this).attr('id'); });
    $('#ajax-comments-submit,#ajax-comments-preview', form).bind('mouseup', function() {
      if (last_submit == $(this).attr('id')) {
        ajax_comments_show_progress(context);
        ajax_comments_update_editors();
      }
    });
    $('#ajax-comments-submit,#ajax-comments-preview', form).bind('keyup', function(event) {
      if (last_submit == $(this).attr('id') && event.keyCode == 13) {
        ajax_comments_show_progress(context);
        ajax_comments_update_editors();
      }
    });
    
    // Enable comments buttons back when attachement is uploaded.
    $('#edit-attach', form).bind('mousedown keydown', function() {
      if (last_submit == $(this).attr('id')) {
        $('#ajax-comments-submit,#ajax-comments-preview', form).removeAttr('disabled');
      }
    });

    // Initializing main form.
    action = form.attr('action');
    
    // Creating title link.
    form.parent('div').prev('h2:not(.ajax-comments-title-processed),h3:not(.ajax-comments-title-processed),h4:not(.ajax-comments-title-processed)').addClass('ajax-comments-title-processed').each(function(){
      var title = $(this).html();
      $(this).html('<a href="' + action + '" id="comment-form-title">' + title + '</a>');
      form.parent('div').attr('id','comment-form-content').removeClass("content");
    });

    // Expanding form if needed.
    page_url = document.location.toString();
    fragment = '';
    if (page_url.match('#')) {
      fragment = page_url.split('#')[1];
    }

    if ((fragment == 'comment-form'  || Drupal.settings.ajax_comments_always_expand_form) && firsttime_init) {
      $('#comment-form-title', context).addClass('pressed');
      $('#comment-form-content').attr('cid', 0);
    }
    else {
      // Fast hide form.
      $('#comment-form-content', context).hide();
    }
    
    // Attaching event to title link.
    $('#comment-form-title:not(.ajax-comments-processed)', context).addClass('ajax-comments-processed').click(Drupal.ajax_comments_reply_click);

    // Moving preview in a proper place.
    if (ajax_comments_is_reply_to_node(action)) {
      $('.ajax-comments-title-processed').before($('#comment-preview'));
    }
    else {
      $('#comment-form-content').before($('#comment-preview'));
    }

    if (!$('#comment-form-content').attr('cid')) {
      $('#comment-form-content').attr('cid', -1);
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
      $(this).click(function(){
        $('#comment-form').attr('action', $(this).attr('href'));
        ajax_comments_reload_form(0, 'pid');

        $('#comment-form-title', context).click();
        ajax_comments_scroll_to_comment_form();
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
 * Comment folding is configured via the AJAX comments administrative page. It enables
 * replies to comments to be 'folded' under the parent.
 */
Drupal.ajax_comments_fold = function(context) {
  $('#comments > .indented:not(.ajax-comments-processed)', context).addClass('folded').addClass('ajax-comments-processed').each(function (){
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
  // We should only handle non pressed links.
  if (!$(this).is('.pressed')){
    action = $(this).attr('href');
    form_action = $('#comment-form').attr('action');
    link_cid = ajax_comments_get_cid_from_href(action);
    rows = Drupal.settings.ajax_comments_rows_default;
    if ($('#comment-form-content').attr('cid') != link_cid) {
      // We should remove any WYSIWYG before moving controls.
      ajax_comments_remove_editors();

      // Move form from old position.
      if (ajax_comments_is_reply_to_node(action)) {
        $('#comment-form-content').removeClass('indented');
        if ($('#comment-form-content:visible').length) {
          $('#comment-form-content').after('<div style="height:' + $('#comment-form-content').height() + 'px;" class="sizer"></div>');
          $('.sizer').slideUp(speed, function(){ $(this).remove(); });
        }
        $(this).parents('h2,h3,h4').after($('#comment-form-content'));
        rows = Drupal.settings.ajax_comments_rows_default;
        $('.ajax-comments-title-processed').before($('#comment-preview'));
      }
      else {
        $('#comment-form-content').addClass('indented');
        if ($('#comment-form-content:visible').length) {
          $('#comment-form-content').after('<div style="height:' + $('#comment-form-content').height() + 'px;" class="sizer"></div>');
          $('.sizer').slideUp(speed, function(){ $(this).remove(); });
        }
        
        folded_thread = $(this).parents(commentbox).next('.indented.folded');
        if (folded_thread.length && Drupal.settings.comment_bonus_api_fold_comments) {
          $(this).parents(commentbox).find('.hide-thread').click();
          folded_thread.after($('#comment-form-content'));
        }
        else {
          $(this).parents(commentbox).after($('#comment-form-content'));
        }
        rows = Drupal.settings.ajax_comments_rows_in_reply;
        $('#comment-form-content').prepend($('#comment-preview'));
      }
      $('#comment-form-content').hide();
    }

    // We don't need to load everything twice.
    if (!$(this).is('.last-clicked')) {
      // Reload form if preview is required.
      if ((Drupal.settings.comment_preview_required && $('#ajax-comments-submit').length) ||
        // Or if quoted comment or custom reload trigger.
        action.match('quote=1') || form_action.match('reload=1')
      ) {
        $('#comment-form').attr('action', action)
        ajax_comments_reload_form(link_cid, 'pid');
        
        $('.editing').fadeTo('fast', 1);
      }
      else {
        ajax_comments_rewind(link_cid, rows);
        ajax_comments_scroll_to_comment_form();
      }
    }
    // ...and show the form after everything is done.
    ajax_comments_expand_form();
    
    $('.pressed').removeClass('pressed');
    $(this).addClass('pressed');
    $('.last-clicked').removeClass('last-clicked');
    $(this).addClass('last-clicked');
    $('#comment-form-content').attr('cid', link_cid);
  }
  else {
    // Handling double click.
    if ((!$(this).is('#comment-form-title')) && (Drupal.settings.ajax_comments_always_expand_form)) {
      $('#comment-form-title').click();
    }
    else if (!Drupal.settings.ajax_comments_always_expand_form) {
      ajax_comments_close_form();
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

  $edit_link.parents(commentbox).fadeTo('fast', 0.5).addClass('editing');

  ajax_comments_show_progress();
  $('#comment-form-content').addClass('indented');
  if ($('#comment-form-content:visible').length) {
    $('#comment-form-content').after('<div style="height:' + $('#comment-form-content').height() + 'px;" class="sizer"></div>');
    $('.sizer').slideUp(speed, function(){ $(this).remove(); });
  }
  
  $edit_link.parents(commentbox).after($('#comment-form-content'));
  rows = Drupal.settings.ajax_comments_rows_in_reply;
  $('#comment-form-content').prepend($('#comment-preview'));
  $('#comment-form-content').hide();
  ajax_comments_expand_form();


  form_action = $('#comment-form').attr('action');
  // Reload form with edit data.
  $args = ajax_comments_get_args(form_action);
  nid = $args[2];
  action = $edit_link.attr('href');
  action = action.replace('comment/edit', 'ajax_comments/js_reload/' + nid) + '/cid';
  ajax_comments_reload_form(action, 'action', function() { 
    // Set reload trigger.
    $('#comment-form').attr('action', form_action + '?reload=1');
    $('#comment-form-content').attr('cid', 'edit');
  });


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
            ajax_comments_close_form();

            // If comment form is expanded on this module, we should collapse it first.
            if (comment.next().is('#comment-form-content')) {
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
                $('#comment-controls').animate({height:'hide', opacity:'hide'}, speed, function(){ $(this).remove(); });
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
 * Attach AHAH behavior to each AHAH form element.
 *
 * Define a function as a property of Drupal.behaviors. This will call the code
 * block when the DOM has finished loading. In this case, we attach the AHAH 
 * behavior to the pager element if it appears on the page. The pager will show
 * up when the number of comments on the page exceeds the "Default comments per page"
 * setting for your node type. 
 */
Drupal.behaviors.ajax_comments_pager = function(context) {
  $('#comments .pager:not(.pager-processed)', context).addClass('pager-processed').each(function() {
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
    ajaxPath = url.replace(/(.*?)\?(.*?)/g, Drupal.settings.basePath + 'ajax_comments/js_load_thread/' + Drupal.settings.ajax_comments_nid + '?$2');
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
function ajax_comments_expand_form(focus) {
  $('#comment-form-content').animate({height:'show'}, speed, function() {
    if (focus) {
      $('#comment-form textarea').focus();
    }
    if ($.browser.msie) this.style.removeAttribute('filter'); 
  });
}

/**
 * Helper function for reply handler.
 *
 * In addition to clearing the textareas and forms, this is where the PID value
 * is set for the comment. The PID controls whether this is a new standalone
 * 'anchor' comment or a reply to an existing comment thread. A PID value
 * of 0 is a new anchor comment while any other number is a reply.
 *
 * @param $pid
 * Some comments are replies to other comments. In those cases, $pid is the parent comment's cid.
 * @param $rows
 *
 */
function ajax_comments_rewind(pid, rows){
  // Resizing and clearing textarea.
  $('#comment-form textarea').attr('rows', rows);
  $('#comment-form:not(.fresh) textarea').attr('value','');

  // Clearing form.
  $('#comment-preview').empty();
  $('#comment-form .error').removeClass('error');

  // Set proper PID.
  $('#comment-form input[name=pid]').val(pid)

  // Now we can attach previously removed editors.
  ajax_comments_attach_editors();
  submit = false;
}

/**
 * Hide comment form, reload if needed.
 */
function ajax_comments_close_form(reload) {
  pid = $('#comment-form-content').attr('cid');
  $('#comment-form-content').animate({height:'hide'}, speed, function(){
    if (reload) {
      ajax_comments_reload_form(pid, 'pid');
    }
  });
  $('.pressed').removeClass('pressed');
  $('#comment-form-content').attr('cid', -1);
  ajax_comments_hide_progress();
}

/**
 * Reload comments form from server.
 */
function ajax_comments_reload_form(id, type, callback) {
  rows = Drupal.settings.ajax_comments_rows_default;
  if (type == 'pid') {
    action = $('#comment-form').attr('action');
    action = action.replace(/comment\/reply\/([0-9]+?)(\/*[0-9]*)$/, 'ajax_comments/js_reload/$1$2');
    action = action.replace(/ajax_comments\/js_reload\/([0-9]+?)\/([0-9]+?)\/cid/, 'ajax_comments/js_reload/$1');

    if (id > 0) {
      action = action.replace(/([?])$/, '/' + id + '?');
      action = action.replace(/#comment-form/, '');
      
      rows = Drupal.settings.ajax_comments_rows_in_reply;
    }
  }
  else if (type == 'action') {
    action = id;
  }

  $('#comment-preview').hide();
  ajax_comments_show_progress();

  $.ajax({
    type: "GET",
    url: action,
    success: function(response) {
     if (response.status && response.content) {
        saved_class = $('#comment-form').attr('class');
        saved_class = saved_class.replace('ajax-comments-processed', '');

        $('#comment-form-content').html(response.content);
        $('#comment-form').attr('class', saved_class);

        $('#comment-form').addClass('fresh');
        Drupal.attachBehaviors($('#comment-form-content'));
        ajax_comments_rewind(id, rows);
        ajax_comments_hide_progress();

        $('#comment-form').removeClass('fresh');

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
function ajax_comments_scroll_to_comment_form() {
  if ($.browser.msie) {
    height = document.documentElement.offsetHeight ;
  }
  else if (window.innerWidth && window.innerHeight) {
    height = window.innerHeight;
  }
  height = height / 2;
  offset = $('#comment-form-content').offset();
  if ((offset.top > $('html').scrollTop() + height) || (offset.top < $('html').scrollTop() - 20)) {
    $('html').animate({scrollTop: offset.top}, 'slow');
  }
}

/**
 * Find a place for a new comment.
 */
function ajax_comments_insert_new_comment($comment) {
  if ($('#comment-form-content').attr('cid') == 'edit') {
    $('#comment-form-content').before($comment);
    $('.editing').remove();
    $('#comment-form-content').attr('cid', 0);
  }
  else if ($('#comment-form-content').attr('cid') <= 0) {
    if ($('#comments .pager').length) {
      $('#comment-preview').prev('.item-list').before($comment);
    }
    else {
      $('#comment-preview').before($comment);
    }
  }
  else {
    if ($('#comment-form-content').next().is('.indented')) {
      $('#comment-form-content').next().append($comment);
    }
    else {
      $('#comment-form-content').before($comment);
      $comment.wrap('<div class="indented"></div>');
    }
  }
}

/**
 * AHAH effect for comment previews.
 *
 * This effect was bound via ajax_comments.module in ajax_comments_form_comment_form_alter()
 * as the AHAH effect.
 *
 * @see http://drupal.org/node/331941
 */
jQuery.fn.ajaxCommentsPreviewToggle = function() {
  var obj = $(this[0]);

  // Hide previous preview.
  $('#comment-preview > div:visible').animate({height:'hide', opacity:'hide'}, speed, function() { $(this).remove(); } );
  // Show fresh preview.
  $('#comment-preview').show();
  obj.animate({height:'show', opacity:'show'}, speed);
  ajax_comments_hide_progress();

  // Add submit button if it doesn't added yet.
  if (!$('#ajax-comments-submit').length && $('.preview-item').length) {
    $('#ajax-comments-preview').after('<input name="op" id="ajax-comments-submit" value="' + Drupal.t("Save") + '" class="form-submit" type="submit">');
    // Re-attaching to new comment.
    Drupal.attachBehaviors($('#ajax-comments-submit'));
  }
};

/**
 * AHAH effect for comment submits.
 *
 * This effect was bound via ajax_comments.module in ajax_comments_form_comment_form_alter()
 * as the AHAH effect. 
 *
 * @see http://drupal.org/node/331941
 */
jQuery.fn.ajaxCommentsSubmitToggle = function() {
  var obj = $(this[0]);

  html = obj.html();
  if (html.indexOf('comment-new-success') > -1) {
    // Empty any preview before output comment.
    $('#comment-preview').slideUp(speed, function(){ $(this).empty(); });

    // If there are more than one page in comments tthread, we should firstly turn over to a last page.
    $last_page = $('#comments .pager:first li.pager-last a');
    if (!$last_page.length) {
      $last_page = $('#comments .pager:first li.pager-next').prev().children('a');
    }
    if ($('#comment-form-content').attr('cid') <= 0 && $last_page.length) {
      Drupal.turn_over_page($('#comments .pager:first'), $last_page.attr('href'), false, function(){}, function(){ $(this).removeClass('throbber'); });
    }
    else {
      // Place new comment in proper place.
      ajax_comments_insert_new_comment(obj);

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
      ajax_comments_close_form(true);

      if (Drupal.settings.ajax_comments_always_expand_form) {
        $('#comment-form-title').click();
      }
    }
  }
  else {
    $('#comment-preview').append(obj);
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
 * Update editors text to their textareas. Needs to be done before submits.
 */
function ajax_comments_update_editors() {
  // Update tinyMCE.
  if (typeof(tinyMCE) != 'undefined') {
    tinyMCE.triggerSave();
  }

  // Update CKeditor.
  if (typeof(CKEDITOR) != 'undefined') {
    Drupal.ckeditorOff('edit-comment');
    Drupal.ckeditorOn("edit-comment");
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

function ajax_comments_is_reply_to_node(href) {
  args = ajax_comments_get_args(href);
  result = args[1] == 'reply' && args[2] && (typeof(args[3]) == 'undefined');
  return result;
}

function ajax_comments_get_args(url) {
  if (Drupal.settings.clean_url == '1') {
    var regexS = "(http(s)*:\/\/)*([^/]*)" + Drupal.settings.basePath + "([^?#]*)";
    var regex = new RegExp( regexS );
    var results = regex.exec( url );
    args = results[4];
  }
  else {
    var regexS = "([&?])q=([^#&?]*)";
    var regex = new RegExp( regexS );
    var results = regex.exec( url );
    args = results[2];
  }
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

/**
 * Display and hide the progress bar.
 *
 * Show or hide the progress bar during the actual comment
 * submit/post. Be careful here with IE7 here as it needs
 * to see the #ajax-comments-submit button or else it won't
 * post.
 */
function ajax_comments_show_progress(context) {
  if (!context) {
    context = '#comment-form-content';
  }
  if (!$('#comment-form .ajax-comments-loader', context).length) {
    $('#comment-form', context).append('<div class="ajax-comments-loader"></div>');
  }
}
function ajax_comments_hide_progress(context) {
  if (!context) {
    context = '#comment-form-content';
  }
  $('#comment-form .ajax-comments-loader', context).fadeOut(speed, function(){ $(this).remove(); });
}
