<?php

/**
 * @file
 * Menu callbacks, form callbacks and helpers.
 */

/**
* Page callback: Product Status Update settings
*
* @see product_status_update_menu()
*/

function product_status_update_form($form, &$form_state) {
	
	$form['batch_size'] = array(
		'#type' => 'textfield',
		'#title' => t('Batch Size'),
		'#default_value' => variable_get('psu_batch_size', 100),
		'#size' => 3,
		'#maxlength' => 3,
		'#description' => t('Number of products to search per batch'),
		'#required' => TRUE,
	);
	
	$form['number_of_batches'] = array(
		'#type' => 'textfield',
		'#title' => t('Batches to Run per cron'),
		'#default_value' => variable_get('psu_batches_per_run', 5),
		'#size' => 2,
		'#maxlength' => 2,
		'#description' => t('Number of batches to process per cron run'),
		'#required' => TRUE,
	);
	
	$form['run_now'] = array(
		'#type' => 'checkbox',
		'#title' => t('Run Test Batch'),
	);
	
	$form['new_inv'] = array(
		'#type' => 'checkbox',
		'#title' => t('Import New Items'),
	);
	
	$form['submit'] = array(
		'#type' => 'submit',
		'#value' => 'Save Configuration',
  );
		
	return $form;
}

/**
* Page callback: Discontinued Products List
*/

function product_status_update_discontinued($form, &$form_state) {
	
	$product_list = '';
	
	$discontinued_products = db_select('product_status_update', 'psu')
		->fields('psu', array('product_id', 'sku'))
		->condition('discontinued', 1)
		->execute()->fetchAllAssoc('product_id', PDO::FETCH_ASSOC);
		
	foreach($discontinued_products as $product) {		
		$title = db_select('commerce_product', 'cp')
			->fields('cp', array('title'))
			->condition('product_id', $product['product_id'])
			->execute()->fetchField();
		
		$product_list .= $product['sku'].' '.$title.'<br />';
	}
	
	$product_list .= '<br />';
	
	$form['report'] = array(
		'#type' => 'item',
		'#title' => 'List of Discontinued Items',
		'#suffix' => $product_list,
	);
	
	$form['submit'] = array(
		'#type' => 'submit',
		'#value' => 'Delete All',
	);
	
	return $form;
}


/**
* Page callback: Out-of-Stock Products List
*/

function product_status_update_outofstock($form, &$form_state) {
	
	$product_list = '';
	
	$outofstock_products = db_query('SELECT * FROM {product_status_update} JOIN {commerce_product} ON product_status_update.product_id = commerce_product.product_id WHERE out_of_stock = 1 AND status = 1')->fetchAllAssoc('product_id', PDO::FETCH_ASSOC);
		
	foreach($outofstock_products as $product) {		
		$title = db_select('commerce_product', 'cp')
			->fields('cp', array('title'))
			->condition('product_id', $product['product_id'])
			->execute()->fetchField();	
			
		$product_list .= $product['sku'].' '.$title.'<br />';
		
	}
	
	$product_list .= '<br />';
	
	$form['report'] = array(
		'#type' => 'item',
		'#title' => 'List of Out-Of-Stock Items',
		'#suffix' => $product_list,
	);
	
	$form['submit'] = array(
		'#type' => 'submit',
		'#value' => 'Disable All',
	);
	
	return $form;
}

/**
* Page callback: Restocked Products List
*/

function product_status_update_restocked($form, &$form_state) {
	
	$product_list = '';
	
	$restocked_products = db_query('SELECT * FROM {product_status_update} JOIN {commerce_product} ON product_status_update.product_id = commerce_product.product_id WHERE out_of_stock = 0 AND status = 0 AND processed = 1')->fetchAllAssoc('product_id', PDO::FETCH_ASSOC);
		
	foreach($restocked_products as $product) {		
		$title = db_select('commerce_product', 'cp')
			->fields('cp', array('title'))
			->condition('product_id', $product['product_id'])
			->execute()->fetchField();	
			
		$product_list .= $product['sku'].' '.$title.'<br />';
		
	}
	
	$product_list .= '<br />';
	
	$form['report'] = array(
		'#type' => 'item',
		'#title' => 'List of Items that are Back in Stock',
		'#suffix' => $product_list,
	);
	
	$form['submit'] = array(
		'#type' => 'submit',
		'#value' => 'Enable All',
	);
	
	return $form;
}

/**
* Page callback: Error Log (Items unable to be processed)
*/

function product_status_update_error_log($form, &$form_state) {
	
	$product_list = '';
	
		$errors = db_select('product_status_update', 'psu')
		->fields('psu', array('product_id', 'sku'))
		->condition('processed', 2)
		->execute()->fetchAllAssoc('product_id', PDO::FETCH_ASSOC);
		
	foreach($errors as $product) {		
		$title = db_select('commerce_product', 'cp')
			->fields('cp', array('title'))
			->condition('product_id', $product['product_id'])
			->execute()->fetchField();	
			
		$product_list .= $product['sku'].' '.$title.'<br />';
		
	}
	
	$product_list .= '<br />';
	
	$form['report'] = array(
		'#type' => 'item',
		'#title' => 'List of Items Unable to be Processed',
		'#suffix' => $product_list,
	);
	
	$form['submit'] = array(
		'#type' => 'submit',
		'#value' => 'Clear Error Log',
	);
	
	return $form;
}


/**
* Validate Settings form data--make sure the user has entered a positive number
*/

function product_status_update_form_validate($form, &$form_state) {
	$size = $form_state['values']['batch_size'];
	$batches = $form_state['values']['number_of_batches'];
	
	if (!is_numeric($size) || $size <= 0) {
		form_set_error('batch_size', t('Please enter a positive number for batch size'));
	}
	if (!is_numeric($batches) || $batches <= 0) {
		form_set_error('number_of_batches', t('Please enter a positive number for batches per run'));
	}
}


/**
* Add a submit handler/function to the config form that will run
* a test batch if specified
*/

function product_status_update_form_submit($form, &$form_state) {
	
	$size = $form_state['values']['batch_size'];
	$batches = $form_state['values']['number_of_batches'];

	variable_set('psu_batch_size', $size);
	variable_set('psu_batches_per_run', $batches);
	
	if ($form_state['values']['run_now']) {
		process_product_batch(1);
	}
	
	if ($form_state['values']['new_inv']) {
		import_new_items();
	}
	
	drupal_set_message(t('Settings have been updated'));
}

/**
* Add a submit handler/function to the Discontinued tab
* Deletes products marked as discontinued
*/

function product_status_update_discontinued_submit($form, &$form_state) {
	
	$number_deleted = 0;
	
	$discontinued_products = db_select('product_status_update', 'psu')
		->fields('psu', array('product_id', 'sku'))
		->condition('discontinued', 1)
		->execute()->fetchAllAssoc('product_id', PDO::FETCH_ASSOC);
		
	foreach ($discontinued_products as $product) {
		commerce_product_delete($product['product_id']);
		db_delete('product_status_update')
				->condition('product_id', $product['product_id'])
				->execute();
		$number_deleted++;
	}
	
	$empty_nodes = db_query("SELECT nid FROM {node} WHERE type = 'product_display' AND NOT EXISTS (SELECT entity_id FROM {field_data_field_product} WHERE nid = entity_id AND field_product_product_id IN (SELECT {product_id} FROM commerce_product))")->fetchAllAssoc('nid', PDO::FETCH_ASSOC);
	
	foreach ($empty_nodes as $node) {
		node_delete($node['nid']);
	}
	
	drupal_set_message(t($number_deleted.' items deleted.'));

}

/**
* Add a submit handler/function to the Out of Stock tab
* Disables products marked as out of stock
*/

function product_status_update_outofstock_submit($form, &$form_state) {
	
	$number_disabled = 0;
	
	$buildable_products = db_query('SELECT * FROM {product_status_update} JOIN {commerce_product} ON product_status_update.product_id = commerce_product.product_id WHERE out_of_stock = 2 AND status = 1')->fetchAllAssoc('product_id', PDO::FETCH_ASSOC);
	
	foreach ($buildable_products as $product) {
		db_update('field_data_field_description')
		->condition('entity_id', $product['product_id'])
		->expression('field_description_value', "REPLACE(field_description_value, 'Particulars', '<em>Please Note: This is a Made Special to Order item.  Please allow 2-4 weeks for delivery.</em><p></p>Particulars')")
		->execute();
		
		db_update('product_status_update')
		->condition('product_id', $product['product_id'])
		->fields(array(
			'out_of_stock' => 3,
			)
		)
		->execute();
	}
	
	$outofstock_products = db_query('SELECT * FROM {product_status_update} JOIN {commerce_product} ON product_status_update.product_id = commerce_product.product_id WHERE out_of_stock = 1 AND status = 1')->fetchAllAssoc('product_id', PDO::FETCH_ASSOC);
		
	foreach ($outofstock_products as $product) {
		db_update('commerce_product')
			->fields(array(
					'status' => 0,
					)
			)
			->condition('product_id', $product['product_id'])
			->execute();
			
			db_update('commerce_product_revision')
			->fields(array(
					'status' => 0,
					)
			)
			->condition('product_id', $product['product_id'])
			->execute();
	
		$number_disabled++;
	}
	
	$empty_nodes = db_query("SELECT nid FROM {node} WHERE type = 'product_display' AND NOT EXISTS (SELECT entity_id FROM {field_data_field_product} WHERE nid = entity_id AND field_product_product_id IN (SELECT {product_id} FROM commerce_product WHERE status = 1))")->fetchAllAssoc('nid', PDO::FETCH_ASSOC);
	
	foreach ($empty_nodes as $node) {
		db_update('node')
			->fields(array(
					'status' => 0,
					)
			)
			->condition('nid', $node['nid'])
			->execute();
			
		db_update('node_revision')
			->fields(array(
					'status' => 0,
					)
			)
			->condition('nid', $node['nid'])
			->execute();
	}
	
	drupal_set_message(t($number_disabled.' items disabled.'));
}

/**
* Add a submit handler/function to the Restocked tab
* Re-enables products that are back in stock
*/

function product_status_update_restocked_submit($form, &$form_state) {
	
	$number_enabled = 0;
	
	$restocked_products = db_query('SELECT * FROM {product_status_update} JOIN {commerce_product} ON product_status_update.product_id = commerce_product.product_id WHERE out_of_stock = 0 AND status = 0 AND processed = 1')->fetchAllAssoc('product_id', PDO::FETCH_ASSOC);
		
	foreach ($restocked_products as $product) {
		db_update('commerce_product')
			->fields(array(
					'status' => 1,
					)
			)
			->condition('product_id', $product['product_id'])
			->execute();
		
		db_update('commerce_product_revision')
			->fields(array(
					'status' => 1,
					)
			)
			->condition('product_id', $product['product_id'])
			->execute();
			
		
		$display_node = db_query('SELECT nid FROM {node} WHERE nid IN (SELECT entity_id FROM field_data_field_product WHERE field_product_product_id ='.$product['product_id'].')')->fetchField();
		
		db_update('node')
			->fields(array(
					'status' => 1,
					)
			)
			->condition('nid', $display_node)
			->execute();
			
		db_update('node_revision')
			->fields(array(
					'status' => 1,
					)
			)
			->condition('nid', $display_node)
			->execute();
	
		$number_enabled++;
	}
	
	drupal_set_message(t($number_enabled.' items re-activated.'));
}

/**
* Add a submit handler/function to the Error Log tab
* Re-queues products with processing errors
*/

function product_status_update_error_log_submit ($form, &$form_state) {
	
	$errors = db_select('product_status_update', 'psu')
	->fields('psu', array('product_id', 'sku'))
	->condition('processed', 2)
	->execute()->fetchAllAssoc('product_id', PDO::FETCH_ASSOC);
	
	foreach ($errors as $product) {
		db_update('product_status_update')
				->fields(array(
					'processed' => 0,
					)
				)
				->condition('sku', $product->sku)
				->execute();
		
	}
	
	drupal_set_message(t('Error Log Cleared'));	
}


