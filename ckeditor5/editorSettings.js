ClassicEditor
				.create( document.querySelector( '.editor' ), {
					
					licenseKey: '',
					link: {
                        addTargetToExternalLinks: false,
                        decorators: {
                            isExternal: {
                                mode: 'automatic',
                                callback: url => false,
                                styles: {
                                    'font-size': '20pt'
                                }
                            }
                        }
                    }
					
				} )
				.then( editor => {
					window.editor = editor;
			
					
					
					
				} )
				.catch( error => {
					console.error( 'Oops, something went wrong!' );
					console.error( 'Please, report the following error on https://github.com/ckeditor/ckeditor5/issues with the build id and the error stack trace:' );
					console.warn( 'Build id: 5xj1mpfbs5mc-7z9gj2ahx10w' );
					console.error( error );
				} );
