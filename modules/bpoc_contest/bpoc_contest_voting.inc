<?php

/**
 * @file bpoc_contest_voting.inc - implements voting api functions for bpoc_contest module
 */

/**
 * save user vote to the database 
 */
function bpoc_contest_vote($nid, $email='', $signup=0) {
  global $user;

  $changed_votes = array();
  $votes = array();
  $vote = array(
    'content_id' => $nid,
    'value' => 1,
    'tag' => 'bpoc_contest',
    'uid' => $user->uid,
    'value_type' => 'points',
  );
  $votes[] = $vote;
  $criteria = array();
  $changed_votes = votingapi_set_votes($votes, $criteria);

  db_query("INSERT INTO {bpoc_contest_votes} (uid, nid, vote_value, email_address, signup, timestamp, source) VALUES (%d, %d, %d, '%s', %d, %d, '%s')", $user->uid, $nid, 1, htmlentities($email), $signup, time(), ip_address());
  rules_invoke_event('bpoc_contest_vote_cast', $user, $nid);
  watchdog('bpoc_contest', 'Vote by @user (@email - @ipaddress) on node @nid.', array('@user' => $user->name, '@email' => $email, '@ipaddress' => ip_address(), '@nid' => $nid));

  return;
}

function bpoc_contest_get_contest_by_submission($nid) {
  $node = node_load($nid); 
  if ($node->field_contest_ref[0]['nid']) {
    return $node->field_contest_ref[0]['nid'];
  }
  return FALSE;
}

function bpoc_contest_votes_by_submission($nid, $email) {
  $votes = db_result(db_query(
           "SELECT COUNT({bpoc_contest_votes}.nid) FROM {bpoc_contest_votes} WHERE {bpoc_contest_votes}.nid = %d AND {bpoc_contest_votes}.email_address = '%s'", 
           $nid, $email));
  return $votes;
}

function bpoc_contest_votes_by_contest($nid, $email) {
  $votes = 0;
  $subs = bpoc_contest_get_submissions($nid);
  foreach ($subs as $sub) {
    $votes = $votes +  bpoc_contest_votes_by_submission($sub, $email);
  }
  return $votes;
}

function bpoc_contest_validate_vote($nid, $email) {
  $subvotes = bpoc_contest_votes_by_submission($nid, $email);
  $contest = bpoc_contest_get_contest_by_submission($nid);
  $convotes = bpoc_contest_votes_by_contest($contest, $email);
  $contest_node = node_load($contest);
// in progress
  return TRUE;
}

function bpoc_contest_latest_vote($nid, $email) {
  $node = node_load($nid);
  if ($node->type == 'contest') {
    $contest = $nid;
    $contest_node = $node;
  }
  else if ($node->type == 'photo_submission') {
    $contest = bpoc_contest_get_contest_by_submission($nid);
    $contest_node = node_load($contest);
  }
  $subs = bpoc_contest_get_submissions($contest);
  $allvotes = array();
  foreach ($subs as $sub) {
    $allvotes = array_merge($allvotes, bpoc_contest_get_votes_by_email($sub, $email));
  }
  return TRUE;
}

/**
 * given a node id, return the vote count for that node
 */
function bpoc_contest_get_submission_vote_count($nid) {
  return db_result(db_query("SELECT COUNT({bpoc_contest_votes}.nid) FROM {bpoc_contest_votes} WHERE {bpoc_contest_votes}.nid = %d", $nid));
}

/*
 * given a contest node id, return an array of submission node ids for that contest
 */
function bpoc_contest_get_submissions($nid) {
  $node = node_load($nid);
  $contest_type = $node->field_contest_type[0]['value'];
  if ($contest_type == 'Photo') {
    $result = db_query("SELECT n.nid FROM {node} n LEFT JOIN {content_field_contest_ref} s ON n.nid = s.nid WHERE s.field_contest_ref_nid = %d", $nid);
  }
  else if ($contest_type == 'Video') {
    $result = db_query("SELECT n.nid FROM {node} n LEFT JOIN {content_field_contest_ref} s ON n.nid = s.nid WHERE s.field_contest_ref_nid = %d", $nid);
  }
  $submissions = array();
  while ($record = db_fetch_object($result)) {
    $submissions[] = $record->nid;
  }
  return $submissions;
}

function bpoc_contest_get_votes($nid) {
  $result = db_query("SELECT v.uid, v.email_address, v.timestamp, v.source FROM {bpoc_contest_votes} v WHERE v.nid = %d", $nid);
  $votes = array();
  $vote = array();
  while ($record = db_fetch_object($result)) {
    $vote['uid'] = $record->uid;
    $vote['email_address'] = $record->email_address;
    $vote['timestamp'] = $record->timestamp;
    $vote['source'] = $record->source;
    $votes[] = $vote;
  }
 return $votes; 
}

function bpoc_contest_get_votes_by_email($nid, $email) {
  $result = db_query("SELECT v.uid, v.email_address, v.timestamp, v.source FROM {bpoc_contest_votes} v WHERE v.nid = %d AND v.email_address = '%s'", $nid, $email);
  $votes = array();
  $vote = array();
  while ($record = db_fetch_object($result)) {
    $vote['uid'] = $record->uid;
    $vote['email_address'] = $record->email_address;
    $vote['timestamp'] = $record->timestamp;
    $vote['source'] = $record->source;
    $votes[] = $vote;
  }
 return $votes; 
}

function bpoc_contest_subs($nid) {
  $node = node_load($nid);
  $contest_type = $node->field_contest_type[0]['value'];
  $result = db_query("SELECT s.nid FROM {content_field_contest_ref} s WHERE s.field_contest_ref_nid = %d", $nid);
  $submissions = array();
  while ($record = db_fetch_object($result)) {
    $submissions[] = $record->nid;
  }
  return $submissions;
}

function bpoc_contest_all_votes($node) {
  $submissions = bpoc_contest_subs($node->nid);
  $header = array('Submission', 'Email', 'Date', 'Source');
  $rows = array();
  foreach($submissions as $nextsub) {
    $submission_node = node_load($nextsub);
    $votes = bpoc_contest_get_votes($nextsub);
    foreach ($votes as $vote) {
      $rows[] = array(
        $submission_node->title . ' (' . $submission_node->nid . '),',
        $vote['email_address'] . ',',
        date('d-M-Y g:ha', $vote['timestamp']) . ',',
        $vote['source'],
      );
    }
  }

  $output = theme('table', $header, $rows);
  return($output);
}


/**
 * menu handler to display votes for both contests and submissions
 */
function bpoc_contest_show_votes($node) {
  if ($node->type == 'contest') {
    return bpoc_contest_show_contest_votes($node);
  }
  if ($node->type == 'photo_submission') {
    return bpoc_contest_show_submission_votes($node);
  }
  if ($node->type == 'video_submission') {
    return bpoc_contest_show_submission_votes($node);
  }
}

/**
 * admin page to display all contest votes
 * will replace with a view when views integration is complete
 */
function bpoc_contest_show_contest_votes($node) {
  $sub_ids = bpoc_contest_get_submissions($node->nid);
  $header = array('Thumbnail', 'Title', 'Votes');
  $rows = array();
  foreach ($sub_ids as $sub) {
    $subnode = node_load($sub);
    $img = $subnode->field_contest_photo[0]['filepath'];
    $alias = drupal_lookup_path('alias', 'node/' . $subnode->nid); 
    if (!$alias) {
      $alias = 'node/' . $subnode->nid;
    }
    $thumb = '<a href="/' . $alias . '">' . 
             theme('imagecache', 'contest_admin_thumbnail', $img) .
             '</a>'; 
    $title = '<a href="/' . $alias . '">' . $subnode->title . '</a>';
    $rows[] = array($thumb, $title, bpoc_contest_get_submission_vote_count($sub));
  }
  array_sort_by_column($rows, '2');
  $output .= theme('table', $header, $rows);

  $output = views_embed_view('bpoc_photo_contest','page_2', $node->nid);


  return $output;
}
/**
 * admin page to display all votes for a submission
 * will replace with a view when views integration is complete
 */
function bpoc_contest_show_submission_votes($node) {
  $votes = bpoc_contest_get_votes($node->nid);
  $header = array('User', 'Email', 'Date', 'Source');
  $rows = array();
  foreach ($votes as $vote) {
    $rows[] = array(
      theme('username', user_load($vote['uid'])),
      $vote['email_address'],
      date('d-M-Y g:ha', $vote['timestamp']),
      $vote['source'],
    );
  }
  $output = theme('table', $header, $rows);
  return $output;
}

// utility functions

/**
 * multi-dimensional array sort
 */
function array_sort_by_column(&$arr, $col, $dir = SORT_DESC) {
  $sort_col = array();
  foreach ($arr as $key=> $row) {
    $sort_col[$key] = $row[$col];
  }
  array_multisort($sort_col, $dir, $arr);
}
