<?php
// $Id: flag_terms.inc,v 1.3 2009/11/19 13:09:02 tayzlor Exp $
/**
 * @file
 * flag_terms.inc
 * Written by Graham Taylor.
 */
 
/**
 * Implements a term flag.
 */
class webform_flag_submissions extends flag_flag {
  function options() {
    $options = parent::options();
    $options += array(
      'show_on_submissions_page' => TRUE,
    );
    return $options;
  }

  function options_form(&$form) {
    parent::options_form($form);

    $options = array();

    // get webform nodes - this works GREAT! woot woot
    $webform_types = webform_variable_get('webform_node_types');

    $nodes = array();
    if ($webform_types) {
      $placeholders = implode(', ', array_fill(0, count($webform_types), "'%s'"));
      $result = db_query(db_rewrite_sql("SELECT n.*, r.* FROM {node} n INNER JOIN {node_revisions} r ON n.vid = r.vid WHERE n.type IN ($placeholders)", 'n', 'nid', $webform_types), $webform_types);
      while ($node = db_fetch_object($result)) {
        $options[$node->nid] = $node->title;
      }
    }

    $form['access']['types'] = array(
      '#type' => 'checkboxes',
      '#title' => t('The webforms whose submissions will be flaggable'),
      '#options' => $options,
      '#default_value' => $this->types,
      '#required' => TRUE,
      '#weight' => 10,
      '#access' => empty($flag->locked['types']),
    );
    $form['display']['show_on_submissions_page'] = array(
      '#type' => 'checkbox',
      '#title' => t('Display link on q=node/x/webform-results pages'),
      '#default_value' => $this->show_on_submissions_page,
      '#access' => empty($this->locked['show_on_submissions_page']),
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