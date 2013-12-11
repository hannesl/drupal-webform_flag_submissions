<?php
 
/**
 * Implements a webform flag submission flag.
 */
class webform_flag_submissions extends flag_flag {
  function options() {
    $options = parent::options();
    $options += array(
      'webform_flag_submissions_show_on_submissions_page' => TRUE,
    );
    return $options;
  }

  function options_form(&$form) {
    parent::options_form($form);

    $options = array();

    // Get webform nodes types.
    $webform_types = webform_variable_get('webform_node_types');

    // Lookup all webform nodes, and add them to the options.
    if ($webform_types && !empty($webform_types)) {
      $nodes = db_select('node', 'n')
        ->fields('n', array('nid', 'title'))
        ->condition('type', $webform_types, 'IN')
        ->execute()
        ->fetchAll();
      if (sizeof($nodes) != 0) {
        foreach($nodes as $node) {
          $options[$node->nid] = $node->title;
        }
      }
      else {
        drupal_set_message(t('There are no webform nodes created!'), 'warning');
      }
    }

    $form['access']['types'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Webforms with Flaggable Submissions'),
      '#options' => $options,
      '#default_value' => $this->types,
      '#required' => TRUE,
      '#weight' => 10,
      '#access' => empty($flag->locked['types']),
    );
    $form['display']['webform_flag_submissions_show_on_submissions_page'] = array(
      '#type' => 'checkbox',
      '#title' => t('Display link on Webform Results Pages (e.g. node/%nid/webform-results)'),
      '#default_value' => $this->webform_flag_submissions_show_on_submissions_page,
      '#access' => empty($this->locked['webform_flag_submissions_show_on_submissions_page']),
    );
  }

  function _load_entity($entity_id) {
    module_load_include('inc', 'webform', 'includes/webform.submissions');
    $submissions = webform_get_submissions(array('sid' => $entity_id));
    return empty($submissions) ? null : reset($submissions);
  }

  function applies_to_entity($submission) {
    if ($submission && in_array($submission->nid, $this->types)) {
      return TRUE;
    }
    return FALSE;
  }

  function get_entity_id($submission) {
    return $submission->sid;
  }

  function get_labels_token_types() {
    return array('submissions');
  }

  function replace_tokens($label, $contexts, $options, $entity_id) {
    if ($entity_id && ($submission = $this->fetch_content($entity_id))) {
      $contexts['submissions'] = $submission;
    }
    return parent::replace_tokens($label, $contexts, $entity_id);
  }

  function get_flag_action($entity_id) {
    $flag_action = parent::get_flag_action($entity_id);
    $submission = $this->fetch_content($entity_id);
    $flag_action->content_title = $submission->title;
    $flag_action->content_url = _flag_url('node/' . $submission->nid . '/webform-results/' . $submission->vid . '/flag');
    return $flag_action;
  }

  function get_relevant_action_objects($entity_id) {
    return array(
      'webform_submission' => $this->fetch_entity($entity_id),
    );
  }

}
