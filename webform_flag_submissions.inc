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

  function _load_content($content_id) {
    $ids = explode('-', $content_id);
    return webform_menu_submission_load($ids[0],$ids[1]);
  }

  function applies_to_content_object($submission) {
    if ($submission && in_array($submission->nid, $this->types)) {
      return TRUE;
    }
    return FALSE;
  }

  function get_content_id($submission) {
    return $submission->sid;
  }

  // The 'Token' module doesn't seem to provide any term tokens, so
  // the following two methods aren't really useful.

  function get_labels_token_types() {
    return array('submissions');
  }

  function replace_tokens($label, $contexts, $content_id) {
    if ($content_id && ($submission = $this->fetch_content($content_id))) {
      $contexts['submissions'] = $submission;
    }
    return parent::replace_tokens($label, $contexts, $content_id);
  }

  function get_flag_action($content_id) {
    $flag_action = parent::get_flag_action($content_id);
    $submission = $this->fetch_content($content_id);
    $flag_action->content_title = $submission->title;
    $flag_action->content_url = _flag_url('node/' . $submission->nid . '/webform-results/' . $submission->vid . '/flag');
    return $flag_action;
  }

  function get_relevant_action_objects($content_id) {
    return array(
      'webform_submission' => $this->fetch_content($content_id),
    );
  }

  function rules_get_event_arguments_definition() {
    return array(
      'term' => array(
        'type' => 'webform_submission',
        'label' => t('flagged submission'),
        'handler' => 'flag_rules_get_event_argument',
      ),
    );
  }

  function rules_get_element_argument_definition() {
    return array('type' => 'webform_submission', 'label' => t('Flagged submission'));
  }

  function applies_to_content_id_array($content_ids) {
    $passed = array();
    foreach ($content_ids as $vid) {
      if ($this->applies_to_content_id($vid)) {
        $passed[$vid] = TRUE;
      }
    }
    return $passed;
  }
}
