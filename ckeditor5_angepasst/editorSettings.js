ClassicEditor
				.create( document.querySelector( '.editor' ), {
					
				toolbar: {
					items: [
						'heading',
						'|',
						'bold',
						'italic',
						'bulletedList',
						'numberedList',
						'|',
						'outdent',
						'indent',
						'|',
						'insertTable',
						'alignment',
						'fontFamily',
						'fontColor',
						'undo',
						'redo',
						'fontBackgroundColor',
						'findAndReplace',
						'fontSize',
						'highlight',
						'horizontalLine',
						'pageBreak',
						'removeFormat',
						'specialCharacters',
						'restrictedEditingException',
						'strikethrough',
						'subscript',
						'superscript',
						'underline'
					]
				},
				language: 'de',
				table: {
					contentToolbar: [
						'tableColumn',
						'tableRow',
						'mergeTableCells',
						'tableCellProperties',
						'tableProperties'
					]
				},
					licenseKey: '',
					
					
					
				} )
				.then( editor => {
					window.editor = editor;
			
					
					
					
				} )
				.catch( error => {
					console.error( 'Oops, something went wrong!' );
					console.error( 'Please, report the following error on https://github.com/ckeditor/ckeditor5/issues with the build id and the error stack trace:' );
					console.warn( 'Build id: 61rgl39c7ch2-8w9swe2au1mp' );
					console.error( error );
				} ); 
