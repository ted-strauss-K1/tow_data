;(function($) {

	$.noty.themes.default = {
		name: 'default',
		helpers: {
			borderFix: function() {
				if (this.options.dismissQueue) {
					var selector = this.options.layout.container.selector + ' ' + this.options.layout.parent.selector;
					switch (this.options.layout.name) {
						case 'top':
							$(selector).css({borderRadius: '0px 0px 0px 0px'});
							$(selector).last().css({borderRadius: '0px 0px 5px 5px'}); break;
						case 'topCenter': case 'topLeft': case 'topRight':
						case 'bottomCenter': case 'bottomLeft': case 'bottomRight':
						case 'center': case 'centerLeft': case 'centerRight': case 'inline':
							$(selector).css({borderRadius: '0px 0px 0px 0px'});
							$(selector).first().css({'border-top-left-radius': '5px', 'border-top-right-radius': '5px'});
							$(selector).last().css({'border-bottom-left-radius': '5px', 'border-bottom-right-radius': '5px'}); break;
						case 'bottom':
							$(selector).css({borderRadius: '0px 0px 0px 0px'});
							$(selector).first().css({borderRadius: '5px 5px 0px 0px'}); break;
						default: break;
					}
				}
			}
		},
		modal: {
			css: {
				position: 'fixed',
				width: '100%',
				height: '100%',
				minHeight: '100%',
				backgroundColor: '#3C3C3C',
				zIndex: 10000,
				opacity: 0.5,
				display: 'none',
				left: 0,
				top: 0
			}
		},
		style: function() {
				
			this.$bar.css({
				overflow: 'hidden'
				
			});
			
			this.$message.css({
				fontSize: '13px',
				fontFamily: 'Helvetica Neue,Helvetica,Arial',
				lineHeight: '1.3em',
				textAlign: 'left',
				padding: '10px 15px',
				width: 'auto',
				color: '#444444',
				position: 'relative'
			});
			
			this.$closeButton.css({
				position: 'absolute',
				top: 4, right: 3,
				width: 10, height: 10,				
                                display: 'block',
				cursor: 'pointer'
			});
			
			this.$buttons.css({
				padding: '8px 5px 10px',
				textAlign: 'right',
				borderTop: '1px solid #ddd',
				backgroundColor: '#fff'
			});
			
			this.$buttons.find('button').css({
				marginLeft: 5
			});
			
			this.$buttons.find('button:first').css({
				marginLeft: 0
			});
      /*
			this.$bar.bind({
				mouseenter: function() { $(this).find('.noty_close').fadeIn(); },
				mouseleave: function() { $(this).find('.noty_close').fadeOut(); }
			});
			*/

			switch (this.options.layout.name) {
				case 'top':
					this.$bar.css({ /*
						borderRadius: '0px 0px 5px 5px',
						borderBottom: '2px solid #eee',
						borderLeft: '2px solid #eee',
						borderRight: '2px solid #eee',
						boxShadow: "0 0 5px rgba(0, 0, 0, 0.5)"
					*/});
				break;
				case 'topCenter': case 'center': case 'bottomCenter': case 'inline':
					this.$bar.css({/*
						borderRadius: '5px',
						border: '1px solid #eee',
						boxShadow: "0 0 5px rgba(0, 0, 0, 0.5)"
					*/});
					this.$message.css({fontSize: '13px', textAlign: 'center', maxWidth: "100%"});
				break;
				case 'topLeft': case 'topRight':
				case 'bottomLeft': case 'bottomRight':
				case 'centerLeft': case 'centerRight':
					this.$bar.css({/*
						borderRadius: '5px',
						border: '1px solid #eee',
						boxShadow: "0 0 5px rgba(0, 0, 0, 0.5)"
					*/});
					this.$message.css({fontSize: '13px', textAlign: 'left'});
				break;
				case 'bottom':
					this.$bar.css({/*
						borderRadius: '5px 5px 0px 0px',
						borderTop: '2px solid #eee',
						borderLeft: '2px solid #eee',
						borderRight: '2px solid #eee',
						boxShadow: "0 0 5px rgba(0, 0, 0, 0.5)"
					*/});
				break;
				default:
					this.$bar.css({/*
						border: '2px solid #eee',
						boxShadow: "0 0 5px rgba(0, 0, 0, 0.5)"
					*/});
				break;
			}
			
			switch (this.options.type) {
				case 'alert': case 'notification':
                                        this.$bar.addClass('alert-note-noty-class'); break;
				case 'warning':
                                        this.$bar.addClass('warning-noty-class');
					this.$buttons.css({borderTop: '1px solid #ddd'}); break;
				case 'error':
                                        this.$bar.addClass('error-noty-class');
					this.$buttons.css({borderTop: '1px solid #ddd'});
                                        break;
				case 'information':
                                        this.$bar.addClass('information-noty-class');
					this.$buttons.css({borderTop: '1px solid #ddd'}); break;
				case 'success':
                                        this.$bar.addClass('success-noty-class');
					this.$buttons.css({borderTop: '1px solid #ddd'});break;
				default:
                                        this.$bar.addClass('default-noty-class'); break;
			}
		},
		callback: {
			onShow: function() { $.noty.themes.default.helpers.borderFix.apply(this); },
			onClose: function() { $.noty.themes.default.helpers.borderFix.apply(this); }
		}
	};

})(jQuery);